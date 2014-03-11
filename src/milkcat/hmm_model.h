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
// hmm_model.h --- Created at 2013-12-05
//

#ifndef SRC_MILKCAT_HMM_MODEL_H_
#define SRC_MILKCAT_HMM_MODEL_H_

#include <string.h>
#include <unordered_map>
#include "utils/status.h"

namespace milkcat {

class HMMModel {
 public:
  struct Emit;

  static HMMModel *New(const char *model_path, Status *status);

  // Save the model to file specified by model_path
  void Save(const char *model_path, Status *status);

  // Create the HMMModel instance from some text model file
  static HMMModel *NewFromText(const char *trans_model_path, 
                               const char *emit_model_path,
                               const char *yset_model_path,
                               const char *index_path,
                               Status *status);
  ~HMMModel();

  int tag_num() const { return tag_num_; }

  // Get Tag's string by its id
  const char *tag_str(int tag_id) const {
    return tag_str_[tag_id];
  }

  // Get tag-id by string
  int tag_id(const char *tag_str) const {
    for (int i = 0; i < tag_num_; ++i) {
      if (strcmp(tag_str_[i], tag_str) == 0)
        return i;
    }

    return -1;
  }

  // Get the emit row (tag, cost) of a term, if no data return nullptr
  Emit *emit(int term_id) const {
    if (term_id > max_term_id_ || term_id < 0)
      return nullptr;
    else
      return emits_[term_id];
  }

  // Get the transition cost from left_tag to right_tag
  float trans_cost(int leftleft_tag, int left_tag, int right_tag) const {
    return transition_matrix_[leftleft_tag * tag_num_ * tag_num_ +
                              left_tag * tag_num_ +
                              right_tag];
  }

  // Get the cost of the probability of single tag
  float tag_cost(int tag_id) const { return tag_cost_[tag_id]; }

 private:
  static constexpr int kTagStrLenMax = 16;
  Emit **emits_;
  int max_term_id_;
  int emit_num_;
  int tag_num_;
  char (* tag_str_)[kTagStrLenMax];
  float *transition_matrix_;
  float *tag_cost_;

  HMMModel();

  static void LoadTransFromText(
      HMMModel *self,
      const char *trans_model_path,
      const std::unordered_map<std::string, int> &y_tag,
      Status *status);

  static void LoadEmitFromText(
      HMMModel *self,
      const char *emit_model_path,
      const char *index_path,
      const std::unordered_map<std::string, int> &y_tag,
      Status *status);

  static std::unordered_map<std::string, int> 
  LoadYTagFromText(HMMModel *self,
                   const char *emit_model_path,
                   Status *status);
};

struct HMMModel::Emit {
  int tag;
  float cost;
  Emit *next;

  Emit(int tag, float cost, Emit *next): tag(tag), cost(cost), next(next) {}
};

}  // namespace milkcat

#endif  // SRC_MILKCAT_HMM_MODEL_H_
