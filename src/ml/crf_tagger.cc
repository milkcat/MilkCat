//
// The MIT License (MIT)
//
// Copyright 2013-2014 The MilkCat Project Developers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Part of this code comes from CRF++
//
// CRF++ -- Yet Another CRF toolkit
// Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
// [original] crf_tagger.cc --- Created at 2013-02-12
// crfpp_tagger.cc --- Created at 2013-10-30
// crf_tagger.cc --- Created at 2013-11-02
//

#include "ml/crf_tagger.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string>
#include "util/util.h"

namespace milkcat {

struct CRFTagger::Node {
  double cost;
  int left_tag_id;
};

const size_t kMaxContextSize = 5;
const char *BOS[kMaxContextSize] = {"_x+1", "_x-2", "_x-3", "_x-4", "_x-#"};
const char *EOS[kMaxContextSize] = {"_x-1", "_x+2", "_x+3", "_x+4", "_x+#"};

CRFTagger::CRFTagger(const CRFModel *model): model_(model) {
  for (int i = 0; i < kSequenceMax; ++i) {
    decode_lattice_[i] = new Node[model_->ysize()];
  }

  transition_table_ = new TransitionTable(model);
  transition_table_->AllowAll();

  lattice_ = new Lattice(model);
  for (int idx = 0; idx < kSequenceMax; ++idx) lattice_->AllowAll(idx);
}

CRFTagger::~CRFTagger() {
  for (int i = 0; i < kSequenceMax; ++i) {
    delete[] decode_lattice_[i];
  }

  delete transition_table_;
  transition_table_ = NULL;

  delete lattice_;
  lattice_ = NULL;
}

void CRFTagger::TagRange(SequenceFeatureSet *sequence_feature_set,
                         int begin,
                         int end,
                         int begin_tag,
                         int end_tag) {
  sequence_feature_set_ = sequence_feature_set;

  Viterbi(begin, end, begin_tag, end_tag);
  StoreResult(begin, end, end_tag);
}

void CRFTagger::Viterbi(int begin, int end, int begin_tag, int end_tag) {
  assert(begin >= 0 && begin < end && end <= sequence_feature_set_->size());
  ClearBucket(begin);
  if (begin_tag != -1) CalcBeginTagBigramCost(begin, begin_tag);
  CalcUnigramCost(begin);

  for (int position = begin + 1; position < end; ++position) {
    // ClearBucket(position);
    CalcBigramCost(position);
    CalcUnigramCost(position);
  }

  if (end_tag != -1) CalcBigramCost(end);
}

void CRFTagger::StoreResult(int begin, int end, int end_tag) {
  int best_yid = 0;
  double best_cost = -1e37;
  const Node *last_bucket = decode_lattice_[end - 1];

  if (end_tag != -1) {
    // Have the end tag ... so find the best result from left tag of `end_tag`
    best_yid = decode_lattice_[end][end_tag].left_tag_id;
  } else {
    int y_num = lattice_->y_num(end - 1);
    for (int y_idx = 0; y_idx < y_num; ++y_idx) {
      int yid = lattice_->at(end - 1, y_idx);
      if (best_cost < last_bucket[yid].cost) {
        best_yid = yid;
        best_cost = last_bucket[yid].cost;
      }
    }
  }

  for (int position = end - 1; position >= begin; --position) {
    result_[position - begin] = best_yid;
    best_yid = decode_lattice_[position][best_yid].left_tag_id;
  }
}

void CRFTagger::ClearBucket(int position) {
  memset(decode_lattice_[position], 0, sizeof(Node) * model_->ysize());
}

void CRFTagger::CalcUnigramCost(int idx) {
  int feature_ids[kMaxFeature],
      feature_id;
  int feature_num = UnigramFeatureAt(idx, feature_ids);
  double cost;

  int y_num = lattice_->y_num(idx);
  for (int y_idx = 0; y_idx < y_num; ++y_idx) {
    int yid = lattice_->at(idx, y_idx);
    cost = decode_lattice_[idx][yid].cost;
    for (int i = 0; i < feature_num; ++i) {
      feature_id = feature_ids[i];
      cost += model_->unigram_cost(feature_id, yid);
    }
    decode_lattice_[idx][yid].cost = cost;
    // printf("Bucket Cost: %d %s %lf\n", idx,
    // model_->GetTagText(yid), cost);
  }
}

void CRFTagger::CalcBeginTagBigramCost(int begin, int begin_tag) {
  int feature_ids[kMaxFeature],
      feature_id;
  int feature_num = BigramFeatureAt(begin, feature_ids);
  double cost;

  int y_num = lattice_->y_num(begin);
  for (int y_idx = 0; y_idx < y_num; ++y_idx) {
    int yid = lattice_->at(begin, y_idx);
    cost = 0;
    for (int i = 0; i < feature_num; ++i) {
      feature_id = feature_ids[i];
      cost += model_->bigram_cost(feature_id, begin_tag, yid);
    }
    decode_lattice_[begin][yid].cost = cost;
  }
}

void CRFTagger::CalcBigramCost(int idx) {
  int feature_ids[kMaxFeature],
      feature_id;
  int feature_num = BigramFeatureAt(idx, feature_ids);
  double cost, best_cost;
  int best_tag_id = 0;

  int y_num = lattice_->y_num(idx);
  int left_y_num = lattice_->y_num(idx - 1);
  for (int y_idx = 0; y_idx < y_num; ++y_idx) {
    int yid = lattice_->at(idx, y_idx);
    best_cost = -1e37;
    for (int left_y_idx = 0; left_y_idx < left_y_num; ++left_y_idx) {
      int left_yid = lattice_->at(idx - 1, left_y_idx);
      // Ignore the bigram when it is not allowed in `transition_table_`
      if (transition_table_->transition(left_yid, yid) == false)
        continue;

      cost = decode_lattice_[idx - 1][left_yid].cost;
      for (int i = 0; i < feature_num; ++i) {
        feature_id = feature_ids[i];
        cost += model_->bigram_cost(feature_id, left_yid, yid);
        // printf("Arc Cost: %s %s %lf\n", model_->GetTagText(left_yid),
        // model_->GetTagText(yid),
        // model_->GetBigramCost(feature_id, left_yid, yid));
      }
      if (cost > best_cost) {
        best_tag_id = left_yid;
        best_cost = cost;
      }
    }
    decode_lattice_[idx][yid].cost = best_cost;
    decode_lattice_[idx][yid].left_tag_id = best_tag_id;
    // printf("Arc Cost: %d %s %s %lf\n", idx, model_->GetTagText(yid),
    // model_->GetTagText(best_tag_id), best_cost);
  }
}

int CRFTagger::UnigramFeatureAt(int position, int *feature_ids) {
  const char *template_str;
  int count = 0,
      feature_id;
  std::string feature_str;
  bool result;

  for (int i = 0; i < model_->unigram_template_num(); ++i) {
    template_str = model_->unigram_template(i);
    result = ApplyRule(&feature_str, template_str, position);
    assert(result);
    feature_id = model_->xid(feature_str.c_str());
    if (feature_id != -1) {
      // printf("%s %d\n", feature_str.c_str(), feature_id);
      feature_ids[count++] = feature_id;
    }
  }

  return count;
}

int CRFTagger::BigramFeatureAt(int position, int *feature_ids) {
  const char *template_str;
  int count = 0,
      feature_id;
  std::string feature_str;
  bool result;

  for (int i = 0; i < model_->bigram_template_num(); ++i) {
    template_str = model_->bigram_template(i);
    result = ApplyRule(&feature_str, template_str, position);
    assert(result);
    feature_id = model_->xid(feature_str.c_str());
    if (feature_id != -1) {
      feature_ids[count++] = feature_id;
    }
  }

  return count;
}

const char *CRFTagger::GetIndex(const char **pp, int position) {
  const char *&p = *pp;
  if (*p++ !='[') {
    return 0;
  }

  int col = 0;
  int row = 0;

  int neg = 1;
  if (*p++ == '-') {
    neg = -1;
  } else {
    --p;
  }

  for (; *p; ++p) {
    switch (*p) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        row = 10 * row +(*p - '0');
        break;
      case ',':
        ++p;
        goto NEXT1;
      default: return  0;
    }
  }

NEXT1:
  for (; *p; ++p) {
    switch (*p) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        col = 10 * col + (*p - '0');
        break;
      case ']': goto NEXT2;
      default: return 0;
    }
  }

NEXT2:
  row *= neg;

  if (row < -static_cast<int>(kMaxContextSize) ||
      row > static_cast<int>(kMaxContextSize) ||
      col < 0) {
    return 0;
  }

  const int idx = position + row;
  if (idx < 0) {
    return BOS[-idx-1];
  }
  if (idx >= static_cast<int>(sequence_feature_set_->size())) {
    return EOS[idx - sequence_feature_set_->size()];
  }

  return sequence_feature_set_->at_index(idx)->at(col);
}

bool CRFTagger::ApplyRule(std::string *output_str,
                          const char *template_str,
                          size_t position) {
  const char *p = template_str,
             *index_str;
  output_str->clear();
  for (; *p; p++) {
    switch (*p) {
      default:
        output_str->push_back(*p);
        break;
      case '%':
        switch (*++p) {
          case 'x':
            ++p;
            index_str = GetIndex(&p, static_cast<int>(position));
            if (!index_str) {
              return false;
            }
            output_str->append(index_str);
            break;
          default:
            return false;
        }
        break;
    }
  }
  return true;
}

CRFTagger::TransitionTable::TransitionTable(const CRFModel *model):
    model_(model) {
  ysize_ = model->ysize();
  transition_ = new bool[ysize_ * ysize_];
  AllowAll();
}
CRFTagger::TransitionTable::~TransitionTable() {
  delete[] transition_;
  transition_ = NULL;
}

void CRFTagger::TransitionTable::Allow(int left, int right) {
  assert(left < ysize_ && right < ysize_);
  transition_[left * ysize_ + right] = true;
}
void CRFTagger::TransitionTable::Disallow(int left, int right) {
  assert(left < ysize_ && right < ysize_);
  transition_[left * ysize_ + right] = false;
}

void CRFTagger::TransitionTable::AllowAll() {
  for (int left = 0; left < ysize_; ++left) {
    for (int right = 0; right < ysize_; ++right) {
      Allow(left, right);
    }
  }
}
void CRFTagger::TransitionTable::DisallowAll() {
  for (int left = 0; left < ysize_; ++left) {
    for (int right = 0; right < ysize_; ++right) {
      Disallow(left, right);
    }
  }
}

CRFTagger::Lattice::Lattice(const CRFModel *model) {
  ysize_ = model->ysize();
  for (int idx = 0; idx < kSequenceMax; ++idx) {
    lattice_[idx] = new int[ysize_];
    top_[idx] = 0;
  }
}

CRFTagger::Lattice::~Lattice() {
  for (int idx = 0; idx < kSequenceMax; ++idx) {
    delete[] lattice_[idx];
    lattice_[idx] = NULL;
  }
}

void CRFTagger::Lattice::Add(int idx, int y) {
  assert(idx < kSequenceMax && top_[idx] < ysize_);
  int top = top_[idx];
  lattice_[idx][top] = y;
  top_[idx]++;
}

void CRFTagger::Lattice::Clear(int idx) {
  top_[idx] = 0;
}

void CRFTagger::Lattice::AllowAll(int idx) {
  for (int i = 0; i < ysize_; ++i) {
    lattice_[idx][i] = i;
  }
  top_[idx] = ysize_;
}

}  // namespace milkcat
