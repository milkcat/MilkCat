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
// beam.h --- Created at 2014-02-25
//

#ifndef SRC_PARSER_BEAM_H_
#define SRC_PARSER_BEAM_H_

#include <assert.h>
#include <stdio.h>
#include <algorithm>
#include "util/util.h"

namespace milkcat {

template<class T, class Comparator>
class Beam {
 public:
  Beam(int beam_size):
      capability_(beam_size * 10),
      beam_size_(beam_size),
      size_(0) {
    items_ = new T *[capability_];
  }

  ~Beam() {
    delete[] items_;
  }

  int size() const { return size_; }
  T *at(int index) const {
    MC_ASSERT(index < beam_size_, "invalid beam index");
    return items_[index];
  }

  // Shrink items_ array and remain top n_best elements
  void Shrink() {
    if (size_ <= beam_size_) return;
    std::partial_sort(items_, items_ + beam_size_, items_ + size_, comparator_);
    size_ = beam_size_;
  }

  // Clear all elements in bucket
  void Clear() {
    size_ = 0;
  }

  // Get minimal node in the bucket
  T *Best() {
    MC_ASSERT(size_ != 0, "beam is empty");
    return *std::min_element(items_, items_ + size_, comparator_);
  }

  // Add an arc to decode graph
  void Add(T *item) {
    items_[size_] = item;
    size_++;
    if (size_ >= capability_) Shrink();
  }

 private:
  T **items_;
  int capability_;
  Comparator comparator_;
  int beam_size_;
  int size_;
};

}  // namespace milkcat

#endif  // SRC_PARSER_BEAM_H_
