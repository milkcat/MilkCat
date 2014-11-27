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
#include "utils/utils.h"

namespace milkcat {

struct CRFTagger::Node {
  double cost;
  int left_tag_id;
};

const size_t kMaxContextSize = 8;
const char *BOS[kMaxContextSize] = { "_B-1", "_B-2", "_B-3", "_B-4",
                                     "_B-5", "_B-6", "_B-7", "_B-8" };
const char *EOS[kMaxContextSize] = { "_B+1", "_B+2", "_B+3", "_B+4",
                                     "_B+5", "_B+6", "_B+7", "_B+8" };

CRFTagger::CRFTagger(const CRFModel *model): model_(model) {
  for (int i = 0; i < kSequenceMax; ++i) {
    lattice_[i] = new Node[model_->ysize()];
  }
}

CRFTagger::~CRFTagger() {
  for (int i = 0; i < kSequenceMax; ++i) {
    delete[] lattice_[i];
  }
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
  if (begin_tag != -1) CalcBeginTagBigramCost(begin_tag);
  CalcUnigramCost(begin);

  for (int position = begin + 1; position < end; ++position) {
    // ClearBucket(position);
    CalcBigramCost(position);
    CalcUnigramCost(position);
  }

  if (end_tag != -1) CalcBigramCost(end);
}

void CRFTagger::StoreResult(int begin, int end, int end_tag) {
  int best_tag_id = 0;
  double best_cost = -1e37;
  const Node *last_bucket = lattice_[end - 1];

  if (end_tag != -1) {
    // Have the end tag ... so find the best result from left tag of `end_tag`
    best_tag_id = lattice_[end][end_tag].left_tag_id;
  } else {
    for (int tag_id = 0; tag_id < model_->ysize(); ++tag_id) {
      if (best_cost < last_bucket[tag_id].cost) {
        best_tag_id = tag_id;
        best_cost = last_bucket[tag_id].cost;
      }
    }
  }

  for (int position = end - 1; position >= begin; --position) {
    result_[position - begin] = best_tag_id;
    best_tag_id = lattice_[position][best_tag_id].left_tag_id;
  }
}

void CRFTagger::ClearBucket(int position) {
  memset(lattice_[position], 0, sizeof(Node) * model_->ysize());
}

void CRFTagger::CalcUnigramCost(int position) {
  int feature_ids[kMaxFeature],
      feature_id;
  int feature_num = UnigramFeatureAt(position, feature_ids);
  double cost;

  for (int tag_id = 0; tag_id < model_->ysize(); ++tag_id) {
    cost = lattice_[position][tag_id].cost;
    for (int i = 0; i < feature_num; ++i) {
      feature_id = feature_ids[i];
      cost += model_->unigram_cost(feature_id, tag_id);
    }
    lattice_[position][tag_id].cost = cost;
    // printf("Bucket Cost: %d %s %lf\n", position,
    // model_->GetTagText(tag_id), cost);
  }
}

void CRFTagger::CalcBeginTagBigramCost(int begin_tag) {
  int feature_ids[kMaxFeature],
      feature_id;
  int feature_num = BigramFeatureAt(0, feature_ids);
  double cost;

  for (int tag_id = 0; tag_id < model_->ysize(); ++tag_id) {
    cost = 0;
    for (int i = 0; i < feature_num; ++i) {
      feature_id = feature_ids[i];
      cost += model_->bigram_cost(feature_id, begin_tag, tag_id);
    }
    lattice_[0][tag_id].cost = cost;
  }
}

void CRFTagger::CalcBigramCost(int position) {
  int feature_ids[kMaxFeature],
      feature_id;
  int feature_num = BigramFeatureAt(position, feature_ids);
  double cost, best_cost;
  int best_tag_id;

  for (int tag_id = 0; tag_id < model_->ysize(); ++tag_id) {
    best_cost = -1e37;
    for (int left_tag_id = 0;
         left_tag_id < model_->ysize();
         ++left_tag_id) {
      cost = lattice_[position - 1][left_tag_id].cost;
      for (int i = 0; i < feature_num; ++i) {
        feature_id = feature_ids[i];
        cost += model_->bigram_cost(feature_id, left_tag_id, tag_id);
        // printf("Arc Cost: %s %s %lf\n", model_->GetTagText(left_tag_id),
        // model_->GetTagText(tag_id),
        // model_->GetBigramCost(feature_id, left_tag_id, tag_id));
      }
      if (cost > best_cost) {
        best_tag_id = left_tag_id;
        best_cost = cost;
      }
    }
    lattice_[position][tag_id].cost = best_cost;
    lattice_[position][tag_id].left_tag_id = best_tag_id;
    // printf("Arc Cost: %d %s %s %lf\n", position, model_->GetTagText(tag_id),
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
      col < 0 || col >= static_cast<int>(model_->xsize())) {
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
            index_str = GetIndex(&p, position);
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

}  // namespace milkcat
