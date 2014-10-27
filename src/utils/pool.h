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
// pool.h --- Created at 2014-04-19
//

#ifndef SRC_UTILS_POOL_H_
#define SRC_UTILS_POOL_H_

#include <vector>

namespace milkcat {

template<class T>
class Pool {
 public:
  Pool(): alloc_index_(0) {}

  ~Pool() {
    for (typename std::vector<T *>::iterator
         it = nodes_.begin(); it < nodes_.end(); ++it) {
      delete *it;
    }
  }

  // Alloc a node
  T *Alloc() {
    if (alloc_index_ == nodes_.size()) {
      nodes_.push_back(new T());
    }
    return nodes_[alloc_index_++];
  }

  // Release all node alloced before
  void ReleaseAll() {
    alloc_index_ = 0;
  }

 private:
  std::vector<T *> nodes_;
  int alloc_index_;
};

}  // namespace milkcat

#endif  // SRC_UTILS_POOL_H_