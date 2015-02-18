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
// hmm_model.h --- Created at 2013-12-05
//

#include "ml/hmm_model.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <map>
#include <string>
#include "common/milkcat_config.h"
#include "common/reimu_trie.h"
#include "util/readable_file.h"
#include "util/util.h"
#include "util/writable_file.h"

namespace milkcat {

HMMModel *HMMModel::New(const char *model_filename, Status *status) {
  HMMModel *self = NULL;
  ReadableFile *fd = ReadableFile::New(model_filename, status);

  // Reads magic number
  int32_t magic_number = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&magic_number, status);
  if (magic_number != kHmmModelMagicNumber) {
    *status = Status::Corruption(model_filename);
  }

  int32_t ysize = 0, xsize = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&ysize, status);
  if (status->ok()) fd->ReadValue<int32_t>(&xsize, status);

  // Reads yname
  char label[kLabelSizeMax];
  std::vector<std::string> yname;
  for (int yid = 0; yid < ysize && status->ok(); ++yid) {
    fd->Read(label, kLabelSizeMax, status);
    yname.push_back(label);
  }

  // Reads transition data
  if (status->ok()) {
    self = new HMMModel(yname);
    self->xsize_ = xsize;
    fd->Read(self->transition_cost_, sizeof(float) * ysize * ysize, status);
  }

  // Reads emission data
  for (int xid = 0; xid < xsize && status->ok(); ++xid) {
    const EmissionArray *emission = EmissionArray::Read(fd, status);
    if (status->ok()) self->emission_.push_back(emission);
  }

  if (status->ok() && fd->Tell() != fd->Size()) {
    *status = Status::Corruption(model_filename);
  }

  delete fd;
  fd = NULL;

  // Reads index file
  std::string index_filename = std::string(model_filename) + ".x.idx";
  if (status->ok()) {
    delete self->index_;
    self->index_ = ReimuTrie::Open(index_filename.c_str());
    if (self->index_ == NULL) {
      *status = Status::IOError(index_filename.c_str());
    }
  }

  if (!status->ok()) {
    delete self;
    return NULL;
  } else {
    return self;
  }
}

void HMMModel::Save(const char *model_filename, Status *status) {
  WritableFile *fd = WritableFile::New(model_filename, status);
  char label[kLabelSizeMax];

  if (status->ok()) fd->WriteValue<int32_t>(kHmmModelMagicNumber, status);
  if (status->ok()) fd->WriteValue<int32_t>(yname_.size(), status);
  if (status->ok()) fd->WriteValue<int32_t>(xsize_, status);

  if (status->ok()) {
    for (int yid = 0;
         yid < static_cast<int>(yname_.size()) && status->ok();
         ++yid) {
      memset(label, 0, sizeof(label));
      strlcpy(label, yname_[yid].c_str(), sizeof(label));
      fd->Write(label, kLabelSizeMax, status);
    }
  }
  if (status->ok()) {
    fd->Write(transition_cost_,
              sizeof(float) * yname_.size() * yname_.size(),
              status);
  }

  // Writes EmissionArray into datafile
  for (int xid = 0; xid < xsize_ && status->ok(); ++xid) {
    emission_[xid]->Write(fd, status);
  }
  
  delete fd;
  fd = NULL;

  // Writes index for word into index file
  std::string index_filename = std::string(model_filename) + ".x.idx";
  if (status->ok()) {
    bool success = index_->Save(index_filename.c_str());
    if (!success) *status = Status::IOError(index_filename.c_str());
  }
}

void HMMModel::AddEmission(const char *word, const EmissionArray &emission) {
  MC_ASSERT(index_->Get(word, -1) == -1, "word already exists");

  index_->Put(word, xsize_);
  ++xsize_;

  // Copy and insert `emission`
  EmissionArray *emission_copy = new EmissionArray(emission);
  emission_.push_back(emission_copy);
}

const HMMModel::EmissionArray *HMMModel::Emission(const char *word) const {
  int xid = index_->Get(word, -1);
  if (xid >= 0) {
    return emission_[xid];
  } else {
    return NULL;
  }
}

HMMModel::HMMModel(const std::vector<std::string> &yname): 
    xsize_(0),
    yname_(yname) {
  index_ = new ReimuTrie();
  transition_cost_ = new float[yname.size() * yname.size()];
}

HMMModel::~HMMModel() {
  delete[] transition_cost_;
  transition_cost_ = NULL;

  delete index_;
  index_ = NULL;

  for (std::vector<const EmissionArray *>::iterator
       it = emission_.begin(); it != emission_.end(); ++it) {
    delete *it;
  }
}

HMMModel::EmissionArray::EmissionArray(int size, int total_count):
    size_(size),
    total_count_(total_count) {
  yid_ = new int[size];
  cost_ = new float[size];
}

HMMModel::EmissionArray::~EmissionArray() {
  delete[] yid_;
  yid_ = NULL;

  delete[] cost_;
  cost_ = NULL;
}

HMMModel::EmissionArray::EmissionArray(const EmissionArray &emission_array):
    size_(emission_array.size_),
    total_count_(emission_array.total_count_) {
  yid_ = new int[size_];
  for (int idx = 0; idx < size_; ++idx) {
    yid_[idx] = emission_array.yid_[idx];
  }
  cost_ = new float[size_];  
  for (int idx = 0; idx < size_; ++idx) {
    cost_[idx] = emission_array.cost_[idx];
  }
}

HMMModel::EmissionArray &HMMModel::EmissionArray::operator=(
    const EmissionArray &emission_array) {
  size_ = emission_array.size_;
  total_count_ = emission_array.total_count_;
  yid_ = new int[size_];
  for (int idx = 0; idx < size_; ++idx) {
    yid_[idx] = emission_array.yid_[idx];
  }
  cost_ = new float[size_];  
  for (int idx = 0; idx < size_; ++idx) {
    cost_[idx] = emission_array.cost_[idx];
  }  
  return *this; 
}

const HMMModel::EmissionArray *HMMModel::EmissionArray::Read(
    ReadableFile *fd,
    Status *status) {
  int32_t magic_number = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&magic_number, status);
  if (status->ok() && magic_number != kMagicNumber) {
    *status = Status::Corruption("Unable to read EmissionArray from file");
  }

  int32_t size = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&size, status);

  int32_t total_count = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&total_count, status);

  EmissionArray *self = NULL;
  if (status->ok()) self = new EmissionArray(size, total_count);

  int32_t yid;
  float cost;
  for (int i = 0; i < size && status->ok(); ++i) {
    fd->ReadValue<int32_t>(&yid, status);
    if (status->ok()) fd->ReadValue<float>(&cost, status);
    if (status->ok()) {
      self->set_yid_at(i, yid);
      self->set_cost_at(i, cost);
    }
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

void HMMModel::EmissionArray::Write(WritableFile *fd, Status *status) const {
  fd->WriteValue<int32_t>(kMagicNumber, status);
  if (status->ok()) fd->WriteValue<int32_t>(size_, status);
  if (status->ok()) fd->WriteValue<int32_t>(total_count_, status);
  for (int i = 0; i < size_ && status->ok(); ++i) {
    fd->WriteValue<int32_t>(yid_[i], status);
    if (status->ok()) fd->WriteValue<float>(cost_[i], status);
  }
}

}  // namespace milkcat
