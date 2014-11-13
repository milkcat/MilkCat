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

#ifndef SRC_PARSER_HMM_MODEL_H_
#define SRC_PARSER_HMM_MODEL_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace milkcat {

class Status;
class ReimuTrie;
class ReadableFile;
class WritableFile;

// HMMModel is a data class for Hidden Markov Model 
class HMMModel {
 public:
  class EmissionArray;

  enum { kBeginOfSenetnceId = 0 };

  HMMModel(const std::vector<std::string> &yname);
  ~HMMModel();

  static HMMModel *New(const char *model_path, Status *status);

  // Save the model to file specified by model_path
  void Save(const char *model_path, Status *status);

  // Label number in this model
  int ysize() const { return yname_.size(); }

  // Get label name by its id
  const char *yname(int yid) const { return yname_[yid].c_str(); }

  // Adds a (word, emission_array) pair into model. It copys the value of
  // `*emission` and insert into the model.
  void AddEmission(const char *word, const EmissionArray &emission);

  // Gets EmissionArray by word. If the word does not exists, return NULL
  const EmissionArray *Emission(const char *word) const;

  // Gets/Sets the transition cost from left_tag to right_tag
  float cost(int left_tag, int right_tag) const {
    return transition_cost_[left_tag * yname_.size() + right_tag];
  }
  void set_cost(int left_tag, int right_tag, float cost) {
    transition_cost_[left_tag * yname_.size() + right_tag] = cost;
  }

 private:
  ReimuTrie *index_;
  std::vector<const EmissionArray *> emission_;
  float *transition_cost_;
  int xsize_;
  std::vector<std::string> yname_;
};

class HMMModel::EmissionArray {
 public:
  enum { kMagicNumber = 0x55 };

  EmissionArray(int size, int total_count);
  ~EmissionArray();
  EmissionArray(const EmissionArray &emission_array);
  EmissionArray &operator=(const EmissionArray &emission_array);

  // Reads an EmissionArray from `fd`
  static const EmissionArray *Read(ReadableFile *fd, Status *status);

  // Writes this EmissionArray to `fd`
  void Write(WritableFile *fd, Status *status) const;

  // Number of emissions
  int size() const { return size_; }

  // Gets/Sets the yid at the `idx` of this array
  int yid_at(int idx) const { return yid_[idx]; }
  void set_yid_at(int idx, int yid) { yid_[idx] = yid; }

  // Gets/Sets the cost at the `idx` of this array
  float cost_at(int idx) const { return cost_[idx]; }
  void set_cost_at(int idx, float cost) { cost_[idx] = cost; }

  // Total count of emissions 
  int total_count() const { return total_count_; }
 
 private:
  int size_;
  int total_count_;
  int *yid_;
  float *cost_;
};

}  // namespace milkcat

#endif  // SRC_PARSER_HMM_MODEL_H_
