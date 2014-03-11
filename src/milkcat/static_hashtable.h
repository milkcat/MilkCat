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
// static_hashtable.h --- Created at 2013-11-14
//


#ifndef SRC_MILKCAT_STATIC_HASHTABLE_H_
#define SRC_MILKCAT_STATIC_HASHTABLE_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include "utils/utils.h"
#include "utils/readable_file.h"
#include "utils/writable_file.h"
#include "utils/status.h"

namespace milkcat {

template <class K, class V>
class StaticHashTable {
 public:
  static const StaticHashTable *Build(const K *keys,
                                      const V *values,
                                      int size,
                                      double load_factor = 0.5) {
    StaticHashTable *self = new StaticHashTable();

    self->bucket_size_ = static_cast<int>(size / load_factor);
    self->buckets_ = new Bucket[self->bucket_size_];
    for (int i = 0; i < self->bucket_size_; ++i) {
      self->buckets_[i].empty = true;
    }

    int position;
    for (int i = 0; i < size; ++i) {
      position = self->FindPosition(keys[i]);
      self->buckets_[position].key = keys[i];
      self->buckets_[position].value = values[i];
      self->buckets_[position].empty = false;
    }

    self->data_size_ = size;
    return self;
  }

  // Load hash table from file
  static const StaticHashTable *New(const char *file_path, Status *status) {
    StaticHashTable *self = new StaticHashTable();
    char *buffer = NULL;

    ReadableFile *fd = ReadableFile::New(file_path, status);

    int32_t magic_number;
    if (status->ok()) fd->ReadValue(&magic_number, status);
    if (status->ok()) {
      if (magic_number != 0x3321) *status = Status::Corruption(file_path);
    }

    int32_t bucket_size;
    if (status->ok()) fd->ReadValue(&bucket_size, status);

    int32_t data_size;
    if (status->ok()) fd->ReadValue(&data_size, status);

    int32_t serialize_size;
    if (status->ok()) fd->ReadValue(&serialize_size, status);

    if (status->ok()) {
      if (kSerializeBukcetSize != serialize_size)
        *status = Status::Corruption(file_path);
    }

    if (status->ok()) {
      buffer = new char[kSerializeBukcetSize * data_size];
      fd->Read(buffer, kSerializeBukcetSize * data_size, status);
    }

    if (status->ok()) {
      if (fd->Tell() != fd->Size()) *status = Status::Corruption(file_path);
    }

    if (status->ok()) {
      self->buckets_ = new Bucket[bucket_size];
      self->bucket_size_ = bucket_size;
      self->data_size_ = data_size;

      for (int i = 0; i < self->bucket_size_; ++i) {
        self->buckets_[i].empty = true;
      }

      char *p = buffer, *p_end = buffer + kSerializeBukcetSize * data_size;
      int32_t position;
      K key;
      V value;
      for (int i = 0; i < data_size; ++i) {
        self->Desericalize(&position, &key, &value, p, p_end - p);
        self->buckets_[position].key = key;
        self->buckets_[position].value = value;
        self->buckets_[position].empty = false;
        p += kSerializeBukcetSize;
      }
    }

    delete[] buffer;
    delete fd;
    fd = NULL;

    if (status->ok()) {
      return self;
    } else {
      delete self;
      return NULL;
    }
  }

  // Save the hash table into file
  void Save(const char *file_path, Status *status) const {
    char buffer[kSerializeBukcetSize];
    WritableFile *fd = WritableFile::New(file_path, status);

    int32_t magic_number = 0x3321;
    if (status->ok()) fd->WriteValue<int32_t>(magic_number, status);
    if (status->ok()) fd->WriteValue<int32_t>(bucket_size_, status);
    if (status->ok()) fd->WriteValue<int32_t>(data_size_, status);

    int32_t serialize_size = kSerializeBukcetSize;
    if (status->ok()) fd->WriteValue<int32_t>(serialize_size, status);

    for (int i = 0; i < bucket_size_ && status->ok(); ++i) {
      if (buckets_[i].empty == false) {
        Serialize(i,
                  buckets_[i].key,
                  buckets_[i].value,
                  buffer,
                  sizeof(buffer));
        fd->Write(buffer, kSerializeBukcetSize, status);
      }
    }

    delete fd;
  }

  // Find the value by key in hash table if exist return a const pointer to the
  // value else return nullptr
  const V *Find(const K &key) const {
    int position = FindPosition(key);
    if (buckets_[position].empty) {
      return NULL;
    } else {
      return &(buckets_[position].value);
    }
  }

  ~StaticHashTable() {
    if (buckets_ != NULL) {
      delete[] buckets_;
      buckets_ = NULL;
    }
  }

 private:
  struct Bucket {
    K key;
    V value;
    bool empty;
  };

  Bucket *buckets_;

  // How many key-value pairs in hash table
  int data_size_;
  int bucket_size_;

  StaticHashTable(): buckets_(NULL), data_size_(0), bucket_size_(0) {}

  // Serialize size of each bucket
  static const int kSerializeBukcetSize = sizeof(int32_t) +
                                          sizeof(K) +
                                          sizeof(V);

  // Serialize a Bucket into buffer
  void Serialize(int32_t position,
                 const K &key,
                 const V &value,
                 void *buffer,
                 int buffer_size) const {
    assert(kSerializeBukcetSize <= buffer_size);

    char *p = reinterpret_cast<char *>(buffer);
    *reinterpret_cast<int32_t *>(p) = position;

    p += sizeof(int32_t);
    *reinterpret_cast<K *>(p) = key;

    p += sizeof(K);
    *reinterpret_cast<V *>(p) = value;
  }

  void Desericalize(int32_t *position,
                    K *key,
                    V *value,
                    const void *buffer,
                    int buffer_size) const {
    assert(kSerializeBukcetSize <= buffer_size);

    const char *p = reinterpret_cast<const char *>(buffer);
    *position = *reinterpret_cast<const int32_t *>(p);

    p += sizeof(int32_t);
    *key = *reinterpret_cast<const K *>(p);

    p += sizeof(K);
    *value = *reinterpret_cast<const V *>(p);
  }

  int FindPosition(const K &key) const {
    int original = JSHash(key) % bucket_size_,
        position = original;
    for (int i = 1; ; i++) {
      if (buckets_[position].empty) {
        // key not exists in hash table
        return position;
      } else if (buckets_[position].key == key) {
        // OK, find the key
        return position;
      } else {
        position = (position + i) % bucket_size_;
        assert(i < bucket_size_);
      }
    }
  }

  // The JS hash function
  int JSHash(const K &key) const {
    const char *p = reinterpret_cast<const char *>(&key);
    uint32_t b = 378551;
    uint32_t a = 63689;
    uint32_t hash = 0;

    for (int i = 0; i < sizeof(K); ++i) {
      hash = hash * a + (*p++);
      a *= b;
    }

    hash = hash & 0x7FFFFFFF;

    return static_cast<int>(hash);
  }


  DISALLOW_COPY_AND_ASSIGN(StaticHashTable);
};

}  // namespace milkcat

#endif  // SRC_MILKCAT_STATIC_HASHTABLE_H_
