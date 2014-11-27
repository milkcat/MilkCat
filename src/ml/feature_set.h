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
// feature_set.h  --- Created at 2014-10-18
//

#ifndef SRC_COMMON_FEATURE_EXTRACTOR_H_
#define SRC_COMMON_FEATURE_EXTRACTOR_H_

#include <assert.h>

namespace milkcat {

// FeatureSet likes a feature vector that each dimemsion is neither 0 or 1.
// A feature in the FeatureSet means the value in this dimension is `1`.
class FeatureSet {
 public:
  FeatureSet();

  enum {
    kFeatureNumberMax = 100,
    kFeatureSizeMax = 128
  };

  // Adds a feature string into the feature set
  void Add(const char *feature_string);

  // Remove all features
  void Clear() { top_ = 0; }

  // Gets the feature string at `index`
  char *at(int index) const {
    assert(index < top_);
    return const_cast<char *>(feature_string_[index]); 
  }

  // Gets number of features in the set
  int size() const { return top_; }
  void set_size(int size) { top_ = size; }

 private:
  char feature_string_[kFeatureNumberMax][kFeatureSizeMax];
  int top_;
};

}  // namespace milkcat

#endif