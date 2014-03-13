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
// Part of this code comes from CRF++
//
// CRF++ -- Yet Another CRF toolkit
// Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
// crfpp_model.cc --- Created at 2013-10-28
// crf_model.cc --- Created at 2013-11-02
//

#include "milkcat/crf_model.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "utils/utils.h"
#include "utils/readable_file.h"

namespace milkcat {

inline const char *read_data(const char **ptr, size_t size) {
  const char *r = *ptr;
  *ptr += size;
  return r;
}

template <class T>
void read_value(const char **ptr, T *value) {
  const char *r = read_data(ptr, sizeof(T));
  memcpy(value, r, sizeof(T));
}

CRFModel::CRFModel(): double_array_(NULL),
                      cost_data_(NULL),
                      cost_num_(0),
                      data_(NULL), 
                      cost_factor_(0.0) {
}

CRFModel::~CRFModel() {
  // cost_data_ and y_ are point to the area of data_
  // so it is unnecessary to delete them

  if (data_ != NULL) {
    delete[] data_;
    data_ = NULL;
  }

  if (double_array_ != NULL) {
    delete double_array_;
    double_array_ = NULL;
  }
}

CRFModel *CRFModel::New(const char *model_path, Status *status) {
  CRFModel *self = new CRFModel();

  ReadableFile *fd = ReadableFile::New(model_path, status);
  if (status->ok()) {
    self->data_ = new char[fd->Size()];
    fd->Read(self->data_, fd->Size(), status);
  }

  const char *ptr, *end;

  if (status->ok()) {
    ptr = self->data_;
    end = ptr + fd->Size();

    int32_t version = 0;
    read_value<int32_t>(&ptr, &version);
    if (version != 100) *status = Status::Corruption(model_path);
  }

  if (status->ok()) {
    int32_t type = 0;
    read_value<int32_t>(&ptr, &type);
    read_value<double>(&ptr, &(self->cost_factor_));
    read_value<int32_t>(&ptr, &(self->cost_num_));
    read_value<int32_t>(&ptr, &(self->xsize_));

    int32_t dsize = 0;
    read_value<int32_t>(&ptr, &dsize);

    int32_t y_str_size;
    read_value<int32_t>(&ptr, &y_str_size);
    const char *y_str = read_data(&ptr, y_str_size);
    int pos = 0;
    while (pos < y_str_size) {
      self->y_.push_back(y_str + pos);
      while (y_str[pos++] != '\0') {}
    }

    int32_t tmpl_str_size;
    read_value<int32_t>(&ptr, &tmpl_str_size);
    const char *tmpl_str = read_data(&ptr, tmpl_str_size);
    pos = 0;
    while (pos < tmpl_str_size) {
      const char *v = tmpl_str + pos;
      if (v[0] == '\0') {
        ++pos;
      } else if (v[0] == 'U') {
        self->unigram_templs_.push_back(v);
      } else if (v[0] == 'B') {
        self->bigram_templs_.push_back(v);
      } else {
        assert(false);
      }
      while (tmpl_str[pos++] != '\0') {}
    }

    self->double_array_ = new Darts::DoubleArray();
    self->double_array_->set_array(const_cast<char *>(ptr));
    ptr += dsize;

    self->cost_data_ = reinterpret_cast<const float *>(ptr);
    ptr += sizeof(float) * self->cost_num_;
  }

  if (status->ok() && ptr != end) {
    *status = Status::Corruption(model_path);
  }

  delete fd;
  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

}  // namespace milkcat
