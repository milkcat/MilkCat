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
// crfpp_model.h --- Created at 2013-10-28
// crf_model.h --- Created at 2013-11-02
//

#ifndef SRC_MILKCAT_CRF_MODEL_H_
#define SRC_MILKCAT_CRF_MODEL_H_

#include <vector>
#include "common/darts.h"
#include "utils/utils.h"

namespace milkcat {

class CRFModel {
 public:
  // Open a CRF++ model file
  static CRFModel *New(const char *model_path, Status *status);

  ~CRFModel();

  // Get internal id of feature_str, if not exists return -1
  int GetFeatureId(const char *feature_str) const {
    return double_array_->exactMatchSearch<int>(feature_str);
  }

  // Get Tag's string text by its id
  const char *GetTagText(int tag_id) const {
    return y_[tag_id];
  }

  // Get Tag's id by its text, return -1 if it not exists
  int GetTagId(const char *tag_text) const  {
    std::vector<const char *>::const_iterator it;
    for (it = y_.begin(); it != y_.end(); ++it) {
      if (strcmp(tag_text, *it) == 0) return it - y_.begin();
    }

    return -1;
  }

  // Get the unigram template
  const char *GetUnigramTemplate(int index) const {
    return unigram_templs_[index];
  }

  // Get the number of unigram template
  int UnigramTemplateNum() const {
    return unigram_templs_.size();
  }

  // Get the bigram template
  const char *GetBigramTemplate(int index) const {
    return bigram_templs_[index];
  }

  // Get the number of unigram template
  int BigramTemplateNum() const {
    return bigram_templs_.size();
  }

  // Get the number of tag
  int GetTagNumber() const {
    return y_.size();
  }

  // Get the cost for feature with current tag
  double GetUnigramCost(int feature_id, int tag_id) const {
    return cost_data_[feature_id + tag_id];
  }

  // Get the bigram cost for feature with left tag and right tag
  double GetBigramCost(int feature_id,
                       int left_tag_id,
                       int right_tag_id) const {
    return cost_data_[feature_id + left_tag_id * y_.size() + right_tag_id];
  }

  // Get feature column number
  int xsize() const { return xsize_; }

 private:
  std::vector<const char *> y_;
  std::vector<const char *> unigram_templs_;
  std::vector<const char *> bigram_templs_;
  Darts::DoubleArray *double_array_;
  const float *cost_data_;
  int cost_num_;
  char *data_;
  double cost_factor_;
  int xsize_;

  CRFModel();
};

}  // namespace milkcat

#endif  // SRC_MILKCAT_CRF_MODEL_H_
