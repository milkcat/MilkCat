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
// instance.h --- Created at 2013-06-09
// instance_data.h --- Created at 2013-10-20
//

#ifndef SRC_COMMON_INSTANCE_DATA_H_
#define SRC_COMMON_INSTANCE_DATA_H_

#include <assert.h>
#include <string.h>
#include "common/milkcat_config.h"
#include "utils/utils.h"

namespace milkcat {

class InstanceData {
 public:
  InstanceData(int string_number, int integer_number, int capability);
  ~InstanceData();

  // Get the string of string_id at the position of this instance
  const char *string_at(int position, int string_id) const {
    assert(position < size_ && string_id < string_number_);
    return string_data_[string_id][position];
  }

  // Set the string of string_id at the position of this instance
  void set_string_at(int position, int string_id, const char *string_val) {
    assert(string_id < string_number_);
    strlcpy(string_data_[string_id][position], string_val, kFeatureLengthMax);
  }

  // Get the integer of integer_id at the position of this instance
  const int integer_at(int position, int integer_id) const {
    assert(position < size_ && integer_id < integer_number_);
    return integer_data_[integer_id][position];
  }

  // Set the integer of integer_id at the position of this instance
  void set_integer_at(int position, int integer_id, int integer_val) {
    assert(integer_id < integer_number_);
    integer_data_[integer_id][position] = integer_val;
  }

  // Get the size of this instance
  int size() const { return size_; }

  // Set the size of this instance
  void set_size(int size) { size_ = size; }

 private:
  char ***string_data_;
  int **integer_data_;
  int string_number_;
  int integer_number_;
  int size_;
  int capability_;

  DISALLOW_COPY_AND_ASSIGN(InstanceData);
};

}  // namespace milkcat

#endif  // SRC_COMMON_INSTANCE_DATA_H_
