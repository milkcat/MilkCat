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
// string_value.h --- Created at 2014-04-10
//

#ifndef SRC_KEYPHRASE_STRING_VALUE_H_
#define SRC_KEYPHRASE_STRING_VALUE_H_

#include <sstream>
#include <string>
#include "common/static_array.h"
#include "common/trie_tree.h"
#include "utils/utils.h"

namespace milkcat {

// StringValueDict is a dict in which the key is string and the value type is T.
// The string could be find in an index file and the value corresponds to the id
// of string in the index.
template <typename T>
class StringValue {
 public:
  StringValue();
  ~StringValue();

  // Creates the dict from text file. On success return the instance of
  // StringValue, on failed, status != Status::OK() and return NULL
  // non_exist_value is the value that could never occur. It indicates that
  // the value in the position doesn't exist.
  static const StringValue<T> *
  NewFromText(const char *filename,
              const TrieTree *index,
              T non_exist_value,
              Status *status);

  // Creates the dict from binary file. On success return the instance of
  // StringValueDict, on failed, status != Status::OK() and return NULL
  static const StringValue<T> *
  New(const char *filename,
      const TrieTree *index,
      T non_exist_value,
      Status *status) {
    StringValue<T> *self = new StringValue<T>();
    self->non_exist_value_ = non_exist_value;
    self->index_ = index;
    self->data_ = StaticArray<T>::New(filename, status);

    if (status->ok()) {
      return self;
    } else {
      delete self;
      return NULL;
    }
  }

  // Gets the value of string. Returns true and saves the value if str exists
  // in the dict. Otherwise, returns false.
  bool Get(const char *str, T *value) const {
    int id = index_->Search(str);
    // fprintf(stderr, "Id is %d\n", id);

    if (id < 0 || id >= data_->size()) return false;

    T val = data_->get(id);
    if (val == non_exist_value_) return false;
    *value = val;
    return true;
  }

  // Save the dict into file. Parameter status indicates whether
  // the dict successfully saved.
  void Save(const char *filename, Status *status) const {
    data_->Save(filename, status);
  }

 private:
  StaticArray<T> *data_;
  const TrieTree *index_;
  T non_exist_value_;
};

template <typename T>
StringValue<T>::StringValue(): data_(NULL), index_(NULL) {
}

template <typename T>
StringValue<T>::~StringValue() {
  delete data_;
  data_ = NULL;
}

template <typename T>
const StringValue<T> *StringValue<T>::NewFromText(
    const char *filename,
    const TrieTree *index,
    T non_exist_value,
    Status *status) {
  char buf[1024];
  utils::unordered_map<int, T> data;
  StringValue<T> *self = new StringValue<T>();

  ReadableFile *fd = ReadableFile::New(filename, status);
  if (status->ok()) fd->ReadLine(buf, sizeof(buf), status);

  std::string word;
  std::istringstream iss;
  int id, max_id = 0;
  T value;
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(buf, sizeof(buf), status);
    if (status->ok()) {
      iss.str(std::string(buf));
      iss >> word >> value;
      id = index->Search(word.c_str());
      if (id > max_id) max_id = id;

      // If the word exists in index
      if (id >= 0) data[id] = value;
    }
  }

  // Copy it to an array
  T *data_array = NULL;
  if (status->ok()) {
    data_array = new T[max_id + 1];

    // Initialize the data_array
    for (int i = 0; i < max_id + 1; ++i) data_array[i] = non_exist_value;

    for (typename utils::unordered_map<int, T>::iterator
         it = data.begin(); it != data.end(); ++it) {
      data_array[it->first] = it->second;
    }

    self->data_ = StaticArray<T>::NewFromArray(data_array, max_id + 1);
    self->index_ = index;
    self->non_exist_value_ = non_exist_value;
  }

  delete fd;
  delete data_array;

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

}  // namespace milkcat

#endif  // SRC_KEYPHRASE_STRING_VALUE_H_