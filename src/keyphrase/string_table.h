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
// string_table.h --- Created at 2014-03-24
//

#ifndef SRC_KEYPHRASE_STRING_TABLE_H_
#define SRC_KEYPHRASE_STRING_TABLE_H_

#include <assert.h>
#include <vector>
#include "utils/utils.h"

namespace milkcat {

class StringTable {
 public:
  StringTable(): size_(0) {
  }

  // Get the id of string. Insert it if the string doesn't exists in
  // StringTable.
  int GetOrInsertId(const char *str) {
    utils::unordered_map<std::string, int>::iterator
    it = string_table_.find(str);

    if (it != string_table_.end()) {
      return it->second;
    } else {
      string_table_[str] = size_;
      string_vector_.push_back(str);
      size_++;
      return size_ - 1;
    }
  }

  // Get string specified by id
  const char *GetStringById(int id) const {
    return string_vector_[id].c_str();
  }

  // Clean all the items in StringTable
  void Clear() {
    string_table_.clear();
    string_vector_.clear();
    size_ = 0;
  }

  int size() const { return size_; }

 private:
  utils::unordered_map<std::string, int> string_table_;
  std::vector<std::string> string_vector_;
  int size_;

  DISALLOW_COPY_AND_ASSIGN(StringTable);
};

}  // namespace milkcat

#endif  // SRC_KEYPHRASE_STRING_TABLE_H_
