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
// maxent_classifier.cc --- Created at 2014-02-01
//

#include "ml/maxent_classifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <algorithm>
#include <map>
#include "common/cedar.h"
#include "utils/readable_file.h"
#include "utils/writable_file.h"
#include "utils/status.h"
#include "utils/log.h"
#include "utils/utils.h"

namespace milkcat {

MaxentModel::MaxentModel(): index_data_(NULL),
                            xsize_(0),
                            ysize_(0),
                            yname_(NULL),
                            cost_(NULL) {
}

MaxentModel *MaxentModel::NewFromText(const char *model_path, Status *status) {
  MaxentModel *self = new MaxentModel();
  char line[1024];

  // The y id and name
  utils::unordered_map<std::string, int> yindex;
  std::vector<std::string> yname;

  // Load maxent data from text model file
  char y[1024], x[1024];
  int xid, yid;
  float cost;

  // First pass - Get x, y names and numbers
  ReadableFile *fd = ReadableFile::New(model_path, status);
  self->xsize_ = 0;
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, 1024, status);
    if (status->ok()) {
      sscanf(line, "%s %s %f", y, x, &cost);
      if (yindex.find(y) == yindex.end()) {
        yindex.insert(std::make_pair(y, yindex.size()));
        yname.push_back(y);
      }

      if (self->index_.exactMatchSearch<int>(x) < 0) {
        self->index_.update(x, strlen(x), self->xsize_);
        ++self->xsize_;
      }
    }
  }

  if (status->ok()) {
    // Generate cost data
    self->ysize_ = yindex.size();
    self->cost_ = new float[self->xsize_ * self->ysize_];
  }

  delete fd;
  fd = NULL;

  if (status->ok()) fd = ReadableFile::New(model_path, status);
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, 1024, status);
    if (status->ok()) {
      sscanf(line, "%s %s %f", y, x, &cost);
      int yid = yindex[y];
      int xid = self->index_.exactMatchSearch<int>(x);

      ASSERT(xid >= 0, "Invalid xid");
      self->cost_[xid * self->ysize_ + yid] = cost;
    }
  }  
  delete fd;
  fd = NULL;


  if (status->ok()) {
    // Generate yname
    self->yname_ = reinterpret_cast<char (*)[kYNameMax]>(
      new char[kYNameMax * self->ysize_]);
    for (int yid = 0; yid < self->ysize_; ++yid) {
      utils::strlcpy(self->yname_[yid], yname[yid].c_str(), kYNameMax);
    }
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

MaxentModel *MaxentModel::New(const char *model_path, Status *status) {
  ReadableFile *fd = ReadableFile::New(model_path, status);
  MaxentModel *self = new MaxentModel();

  if (status->ok()) {
    int magic_number = 0;
    fd->ReadValue<int32_t>(&magic_number, status);
    if (magic_number != 0x2233) *status = Status::Corruption(model_path);
  }

  if (status->ok()) fd->ReadValue<int32_t>(&(self->xsize_), status);
  if (status->ok()) fd->ReadValue<int32_t>(&(self->ysize_), status);
  if (status->ok()) {
    self->yname_ = reinterpret_cast<char (*)[kYNameMax]>(
      new char[kYNameMax * self->ysize_]);
    fd->Read(self->yname_, kYNameMax * self->ysize_, status);
  }

  int index_size = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&index_size, status);
  if (status->ok()) {
    self->index_data_ = new char[index_size];
    fd->Read(self->index_data_, index_size, status);
    if (status->ok()) self->index_.set_array(self->index_data_);
  }

  if (status->ok()) {
    self->cost_ = new float[self->xsize_ * self->ysize_];
    fd->Read(self->cost_, sizeof(float) * self->xsize_ * self->ysize_, status);
  }

  delete fd;
  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

// Maxent file struct
//
// int32_t magic_number = 0x2233
// int32_t xsize
// int32_t ysize
// char[ysize * kYNameMax] yname
// int32_t index_size
// char[index_size] index
// float[xsize * ysize] cost
void MaxentModel::Save(const char *model_path, Status *status) {
  WritableFile *fd = WritableFile::New(model_path, status);

  if (status->ok()) {
    int32_t magic_number = 0x2233;
    fd->WriteValue<int32_t>(magic_number, status);
  }

  if (status->ok()) fd->WriteValue<int32_t>(xsize_, status);
  if (status->ok()) fd->WriteValue<int32_t>(ysize_, status);
  if (status->ok()) fd->Write(yname_, ysize_ * kYNameMax, status);
  if (status->ok())
    fd->WriteValue<int32_t>(
      static_cast<int32_t>(index_.total_size()),
      status);
  if (status->ok()) fd->Write(index_.array(),
                              index_.total_size(),
                              status);
  if (status->ok()) fd->Write(cost_, sizeof(float) * xsize_ * ysize_, status);

  delete fd;
}

MaxentModel::~MaxentModel() {
  delete[] cost_;
  cost_ = NULL;

  delete[] yname_;
  yname_ = NULL;

  delete[] index_data_;
  index_data_ = NULL;
}

MaxentClassifier::MaxentClassifier(const MaxentModel *model):
    model_(model),
    y_cost_(new double[model->ysize()]),
    y_size_(model->ysize()) {
}

MaxentClassifier::~MaxentClassifier() {
  delete[] y_cost_;
  y_cost_ = NULL;
}

int MaxentClassifier::Classify(const char **x, int xsize) const {
  // Clear the y_cost_ array
  for (int i = 0; i < y_size_; ++i) y_cost_[i] = 0.0;

  for (int i = 0; i < xsize; ++i) {
    int xid = model_->feature_id(x[i]);
    if (xid != MaxentModel::kFeatureIdNone) {
      // If this feature exists
      for (int yid = 0; yid < y_size_; ++yid) {
        LOG("Fearure:" << x[i] << " " << yname(yid) << " " << "Cost:" << " "
            << model_->cost(yid, xid));
        y_cost_[yid] += model_->cost(yid, xid);
      }
    }
  }

  for (int i = 0; i < y_size_; ++i) {
    LOG("Y: " << yname(i) << " " << y_cost_[i]);
  }

  // To find the maximum cost of y
  double *maximum_y = std::max_element(y_cost_, y_cost_ + y_size_);
  int maximum_yid = maximum_y - y_cost_;
  return maximum_yid;
}

}  // namespace milkcat