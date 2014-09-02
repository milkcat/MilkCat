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
#include "ml/sequence_feature_extractor.h"

namespace milkcat {

class CRFTagger {
 public:
  explicit CRFTagger(const CRFModel *model);
  ~CRFTagger();
  static const int kMaxBucket = kTokenMax;
  static const int kMaxFeature = 24;

  // Tag a range of instance with the begin tag before the result and the end
  // tag after the result
  void TagRange(SequenceFeatureExtractor *feature_extractor,
                int begin,
                int end,
                int begin_tag,
                int end_tag);

  // Tag a range of instance
  void TagRange(SequenceFeatureExtractor *feature_extractor, int begin, int end) {
    TagRange(feature_extractor, begin, end, -1, -1);
  }

  // Tag a sentence the result could retrive by GetTagAt
  void Tag(SequenceFeatureExtractor *feature_extractor) {
    TagRange(feature_extractor, 0, feature_extractor->size(), -1, -1);
  }

  // Get the tag probability at one position in instance, only use the unigram
  // feature write the result into probability (probability of tag at
  // probability[tag]).
  void ProbabilityAtPosition(SequenceFeatureExtractor *feature_extractor, 
                             int position,
                             double *probability);

  // Get the result tag at position, position starts from 0
  int GetTagAt(int position) {
    return result_[position];
  }

  // Get the number of tags in model
  int GetTagSize() const {
    return model_->GetTagNumber();
  }

  // Get Tag's id by its text, return -1 if it not exists
  int GetTagId(const char *tag_text) const {
    return model_->GetTagId(tag_text);
  }

  // Get Tag's string text by its id
  const char *GetTagText(int tag_id) {
    return model_->GetTagText(tag_id);
  }

 private:
  struct Node;

  const CRFModel *model_;
  Node *buckets_[kMaxBucket];
  int result_[kMaxBucket];
  SequenceFeatureExtractor *feature_extractor_;

  char feature_cache_[kMaxBucket][kMaxFeature][kFeatureLengthMax];
  int feature_cache_left_;
  int feature_cache_right_;
  bool feature_cache_flag_[kMaxBucket];

  // Get the feature from cache or feature extractor
  const char *GetFeatureAt(int position, int index);

  // Clear the Feature cache
  void ClearFeatureCache();

  // Get the id list of bigram features in position, returns the size of the
  // list
  int GetBigramFeatureIds(int position, int *feature_ids);

  // Get the id list of unigram features in position, returns the size of the
  // list
  int GetUnigramFeatureIds(int position, int *feature_ids);

  // CLear the decode bucket
  void ClearBucket(int position);

  // Calculate the unigram cost for each tag in bucket
  void CalculateBucketCost(int position);

  // Calcualte the bigram cost from tag to tag in bucket
  void CalculateArcCost(int position);

  // Calculate the cost of the arc from begin tag to all tags in position 0
  void CalculateBeginTagArcCost(int begin_tag);

  // Viterbi algorithm
  void Viterbi(int begin, int end, int begin_tag, int end_tag);

  // Get the best tag sequence from Viterbi result
  void FindBestResult(int begin, int end, int end_tag);

  const char *GetIndex(const char **pp, int position);
  bool ApplyRule(std::string *output_str,
                 const char *template_str,
                 size_t position);
};

}  // namespace milkcat

#endif  // SRC_PARSER_CRF_TAGGER_H_
