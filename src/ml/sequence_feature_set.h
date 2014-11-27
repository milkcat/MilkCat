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
// sequence_feature_set.h --- Created at 2014-11-27
//

#ifndef SRC_ML_SEQUENCE_FEATURE_SET_H_
#define SRC_ML_SEQUENCE_FEATURE_SET_H_

#include <assert.h>
#include "common/milkcat_config.h"
#include "ml/feature_set.h"

namespace milkcat {

// A sequence of FeatureSet
class SequenceFeatureSet {
 public:
  SequenceFeatureSet(): size_(0) {
  }

  // Returns the FeatureSet at `index`
  FeatureSet *at_index(int index) {
    assert(index < size_);
    return &sequence_[index];
  }

  // Sets/Gets the size of current SequenceFeatureSet
  void set_size(int size) {
    assert(size_ < kTokenMax);
    size_ = size;
  }
  int size() const { return size_; }


 private:
  FeatureSet sequence_[kTokenMax];
  int size_;
};

}  // namespace milkcat

#endif  // SRC_ML_SEQUENCE_FEATURE_SET_H_