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
// [original] crf_tagger.h --- Created at 2013-02-12
// crfpp_tagger.h --- Created at 2013-10-30
// crf_tagger.h --- Created at 2013-11-02
//

#ifndef SRC_PARSER_CRF_TAGGER_H_
#define SRC_PARSER_CRF_TAGGER_H_

#include <string>
#include "common/milkcat_config.h"
#include "ml/crf_model.h"
#include "ml/sequence_feature_set.h"

namespace milkcat {

class SequenceFeatureSet;

class CRFTagger {
 public:
  explicit CRFTagger(const CRFModel *model);
  ~CRFTagger();
  static const int kMaxFeature = 24;

  // Tag a range of instance with the begin tag before the result and the end
  // tag after the result
  void TagRange(SequenceFeatureSet *sequence_feature_set,
                int begin,
                int end,
                int begin_tag,
                int end_tag);

  // Tag a range of instance
  void TagRange(SequenceFeatureSet *sequence_feature_set, int begin, int end) {
    TagRange(sequence_feature_set, begin, end, -1, -1);
  }

  // Tag a sentence the result could retrive by GetTagAt
  void Tag(SequenceFeatureSet *sequence_feature_set) {
    TagRange(sequence_feature_set, 0, sequence_feature_set->size(), -1, -1);
  }

  // Get the result tag at `idx`, position starts from 0
  int y(int idx) {
    return result_[idx];
  }

  // Get the number of tags in model
  int ysize() const {
    return model_->ysize();
  }

  // Get Tag's id by its text, return -1 if it not exists
  int yid(const char *yname) const {
    return model_->yid(yname);
  }

  // Get Tag's string text by its id
  const char *GetTagText(int tag_id) {
    return model_->yname(tag_id);
  }

 private:
  struct Node;

  const CRFModel *model_;
  Node *lattice_[kSequenceMax];
  int result_[kSequenceMax];
  SequenceFeatureSet *sequence_feature_set_;

  // Get the xid of unigram/bigram features at `idx`, returns the number of
  // features
  int BigramFeatureAt(int idx, int *feature_ids);
  int UnigramFeatureAt(int idx, int *feature_ids);

  // CLear the decode bucket
  void ClearBucket(int position);

  // Calculate the unigram/bigram costs
  void CalcUnigramCost(int idx);
  void CalcBigramCost(int idx);
  void CalcBeginTagBigramCost(int begin_tag);

  // Viterbi algorithm
  void Viterbi(int begin, int end, int begin_tag, int end_tag);

  // Get the best tag sequence from lattice and stores it into `lattice_`
  void StoreResult(int begin, int end, int end_tag);

  const char *GetIndex(const char **pp, int position);
  bool ApplyRule(std::string *output_str,
                 const char *template_str,
                 size_t position);
};

}  // namespace milkcat

#endif  // SRC_PARSER_CRF_TAGGER_H_
