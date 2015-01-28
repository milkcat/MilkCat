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
#include "util/readable_file.h"
#include "util/status.h"
#include "util/writable_file.h"

namespace milkcat {

template<class T>
class PackedScore {
 private:
  class IndexData {
   public:
    IndexData(): index(0), data(T()) {}
    IndexData(int index, T data): index(index), data(data) {}
    int32_t index;
    T data;
    bool operator<(const IndexData &right) const {
      return index < right.index;
    }
  };

 public:
  typedef int Iterator;
  enum { kMagicNumber = 0x5a };

  PackedScore(): data_(NULL), size_(0) {
  }

  ~PackedScore() {
    free(data_);
    data_ = NULL;
  }

  // Initialize the iterator
  void Begin(Iterator *it) {
    *it = 0;
  }

  // Gets the (index, value) pair and move iterator to next
  std::pair<int, T> Next(Iterator *it) {
    ++*it;
    IndexData id = data_[*it - 1];
    return std::pair<int, T>(id.index, id.data);
  }

  // Returns true if the iterator has next value
  bool HasNext(Iterator it) const {
    return it < size();
  }

  // Puts value into PackedScore, just like `score[index] = value`
  void Put(int index, T value) {
    T *val = GetOrInsertByIndex(index);
    *val = value;
  }

  void Add(int index, T value) {
    T *val = GetOrInsertByIndex(index);
    *val += value;
  }

  // Gets value by index 
  T Get(int index) const {
    if (data_ == NULL) return 0;

    int idx = BinarySearch(index);
    if (data_[idx].index == index) {
      return data_[idx].data;
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
      self = new PackedScore<T>();
      self->size_ = size;
      self->data_ = static_cast<IndexData *>(malloc(sizeof(IndexData) * size));
    }

    int32_t index;
    T value;
    for (int i = 0; i < size && status->ok(); ++i) {
      fd->ReadValue<int32_t>(&index, status);
      if (status->ok()) fd->ReadValue<float>(&value, status);
      if (status->ok()) {
        self->data_[i] = IndexData(index, value);
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
    if (status->ok()) fd->WriteValue<int32_t>(0, status);
    for (int i = 0; i < size() && status->ok(); ++i) {
      fd->WriteValue<int32_t>(data_[i].index, status);
      if (status->ok()) fd->WriteValue<T>(data_[i].data, status);
    }
  }

  // Number of emissions
  int size() const {
    return size_;
  }
 
 private:
  int size_;
  IndexData *data_;

  // Binary searches `data_` find the matched index
  int BinarySearch(int index) {
    IndexData *p = std::lower_bound(
        data_, data_ + size_, IndexData(index, T()));
    return p - data_;
  }

  T *GetOrInsertByIndex(int index) {
    int idx = BinarySearch(index);

    // Inserts the index if does not exist
    if (idx >= size_ || data_[idx].index != index) {
      data_ = static_cast<IndexData *>(
          realloc(data_, sizeof(IndexData) * (size_ + 1)));
      for (int i = size_ - 1; i >= idx; --i) {
        data_[i + 1] = data_[i];
      }
      data_[idx] = IndexData(index, T());
      ++size_;
    }
    return &data_[idx].data; 
  }
};

}

#endif  // SRC_ML_PACKED_SCORE_H_