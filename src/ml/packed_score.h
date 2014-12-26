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
// packed_score.h --- Created at 2014-12-26
//

#ifndef SRC_ML_PACKED_SCORE_H_
#define SRC_ML_PACKED_SCORE_H_

#include <assert.h>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <utility>
#include "utils/readable_file.h"
#include "utils/status.h"
#include "utils/writable_file.h"

namespace milkcat {

template<class T>
class PackedScore {
 public:
  typedef int Iterator;
  enum { kMagicNumber = 0x5a };

  PackedScore(int total_count): total_count_(total_count) {
  }

  // Initialize the iterator
  void Begin(Iterator *it) {
    *it = 0;
  }

  // Gets the (index, value) pair and move iterator to next
  std::pair<int, T> Next(Iterator *it) {
    ++*it;
    return std::pair<int, T>(index_[*it - 1], data_[*it - 1]);
  }

  // Returns true if the iterator has next value
  bool HasNext(Iterator it) const {
    return it < size();
  }

  // Puts value into PackedScore, just like `score[index] = value`
  void Put(int index, T value) {
    T *val = GetOrInsertByRef(index);
    *val = value;
  }

  void Add(int index, T value) {
    T *val = GetOrInsertByRef(index);
    *val += value;
  }

  // Gets value by index 
  T Get(int index) const {
    std::vector<int>::iterator
    low = std::lower_bound(index_.begin(), index_.end(), index);
    int position = low - index_.begin();
    if (*low == index) {
      return data_[position];
    } else {
      return T();
    }
  }

  // Reads an PackedScore from `fd`
  static PackedScore *Read(ReadableFile *fd, Status *status) {
    int32_t magic_number = 0;
    if (status->ok()) fd->ReadValue<int32_t>(&magic_number, status);
    if (status->ok() && magic_number != kMagicNumber) {
      *status = Status::Corruption("Unable to read PackedScore from file");
    }

    int32_t size = 0;
    if (status->ok()) fd->ReadValue<int32_t>(&size, status);

    int32_t total_count = 0;
    if (status->ok()) fd->ReadValue<int32_t>(&total_count, status);

    PackedScore *self = NULL;
    if (status->ok()) {
      self = new PackedScore<T>(total_count);
      self->index_.resize(size);
      self->data_.resize(size);
    }

    int32_t index;
    T value;
    for (int i = 0; i < size && status->ok(); ++i) {
      fd->ReadValue<int32_t>(&index, status);
      if (status->ok()) fd->ReadValue<float>(&value, status);
      if (status->ok()) {
        self->index_[i] = index;
        self->data_[i] = value;
      }
    }

    if (status->ok()) {
      return self;
    } else {
      delete self;
      return NULL;
    }
  }

  // Writes this PackedScore to `fd`
  void Write(WritableFile *fd, Status *status) const {
    fd->WriteValue<int32_t>(kMagicNumber, status);
    if (status->ok()) fd->WriteValue<int32_t>(size(), status);
    if (status->ok()) fd->WriteValue<int32_t>(total_count_, status);
    for (int i = 0; i < size() && status->ok(); ++i) {
      fd->WriteValue<int32_t>(index_[i], status);
      if (status->ok()) fd->WriteValue<float>(data_[i], status);
    }
  }

  // Number of emissions
  int size() const {
    assert(index_.size() == data_.size());
    return index_.size();
  }

  // Total count of emissions 
  int total_count() const { return total_count_; }
 
 private:
  int total_count_;
  std::vector<int> index_;
  std::vector<T> data_;

  T *GetOrInsertByRef(int index) {
    std::vector<int>::iterator
    low = std::lower_bound(index_.begin(), index_.end(), index);
    int position = low - index_.begin();
    if (low == index_.end() || *low != index) {
      index_.insert(low, index);
      typename std::vector<T>::iterator data_low = data_.begin() + position;
      data_.insert(data_low, T());
    }
    return &data_[position]; 
  }
};

}

#endif  // SRC_ML_PACKED_SCORE_H_