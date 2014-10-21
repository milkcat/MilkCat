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
// multiclass_perceptron_model.cc --- Created at 2014-10-19
//

#define DEBUG

#include "ml/multiclass_perceptron_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <algorithm>
#include <map>
#include <set>
#include "common/milkcat_config.h"
#include "common/reimu_trie.h"
#include "utils/readable_file.h"
#include "utils/writable_file.h"
#include "utils/status.h"
#include "utils/log.h"
#include "utils/utils.h"

namespace milkcat {

MulticlassPerceptronModel::MulticlassPerceptronModel(
    const std::vector<std::string> &y):
        xsize_(0),
        yname_(y) {
  xindex_ = new ReimuTrie();
  yindex_ = new ReimuTrie();
  int yid = 0;
  for (std::vector<std::string>::const_iterator
       it = y.begin(); it != y.end(); ++it) {
    yindex_->Put(it->c_str(), yid++);
  }
}

MulticlassPerceptronModel *
MulticlassPerceptronModel::OpenText(const char *filename, Status *status) {
  // Load maxent data from text model file
  char y[1024], x[1024], line[1024];
  ReimuTrie::int32 xid, yid;
  int xsize = 0;
  float cost;

  // Get y number
  ReadableFile *fd = ReadableFile::New(filename, status);
  std::set<std::string> yname_set;
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, 1024, status);
    if (status->ok()) {
      sscanf(line, "%s %s %f", y, x, &cost);
      if (yname_set.find(y) == yname_set.end()) yname_set.insert(y);
    }
  }
  delete fd;
  fd = NULL;

  MulticlassPerceptronModel *self = NULL;
  if (status->ok()) {
    std::vector<std::string> yname(yname_set.begin(), yname_set.end());
    self = new MulticlassPerceptronModel(yname);
  }

  // Get x, y cost
  bool exist;
  if (status->ok()) fd = ReadableFile::New(filename, status);
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, 1024, status);
    if (status->ok()) {
      sscanf(line, "%s %s %f", y, x, &cost);
      exist = self->yindex_->Get(y, &yid);
      ASSERT(exist, "yindex corrupted");
      xid = self->GetOrInsertXId(x);
      self->cost_[xid * self->ysize() + yid] = cost;
    }
  }  
  delete fd;
  fd = NULL;

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

MulticlassPerceptronModel *
MulticlassPerceptronModel::Open(const char *filename_prefix, Status *status) {
  std::string prefix = filename_prefix;
  std::string metafile = prefix + ".meta";
  std::string xindex_file = prefix + ".x.idx";
  std::string cost_file = prefix + ".cost.data";

  // The metadata file
  ReadableFile *fd = ReadableFile::New(metafile.c_str(), status);
  MulticlassPerceptronModel *self = NULL;

  if (status->ok()) {
    int magic_number = 0;
    fd->ReadValue<int32_t>(&magic_number, status);
    if (magic_number != kMulticlassPerceptronModelMagicNumber) {
      *status = Status::Corruption(metafile.c_str());
    }
  }

  int32_t ysize;
  int32_t xsize;
  int32_t xindex_size;
  char (*yname_buf)[kLabelSizeMax] = NULL;
  std::vector<std::string> yname;
  if (status->ok()) fd->ReadValue<int32_t>(&xsize, status);
  if (status->ok()) fd->ReadValue<int32_t>(&ysize, status);
  if (status->ok()) fd->ReadValue<int32_t>(&xindex_size, status);
  if (status->ok()) {
    yname_buf = reinterpret_cast<char (*)[kLabelSizeMax]>(
        new char[kLabelSizeMax * ysize]);
    fd->Read(yname_buf, kLabelSizeMax * ysize, status);
  }
  if (status->ok()) {
    for (int yid = 0; yid < ysize; ++yid) {
      yname.push_back(yname_buf[yid]);
    }
    self = new MulticlassPerceptronModel(yname);
    self->xsize_ = xsize;
  }
  delete fd;

  // The x-index file (ReimuTrie)
  if (status->ok()) {
    // Use new xindex instead
    delete self->xindex_;
    self->xindex_ = ReimuTrie::Open(xindex_file.c_str());
    if (self->xindex_ == NULL) *status = Status::IOError(xindex_file.c_str());
  }

  if (status->ok()) {
    if (self->xindex_->size() != xindex_size) {
      *status = Status::Corruption(xindex_file.c_str());
    }
  }

  // Cost data file
  if (status->ok()) fd = ReadableFile::New(cost_file.c_str(), status);
  if (status->ok()) {
    int cost_size = self->xsize_ * self->ysize();
    self->cost_.resize(cost_size);
    fd->Read(&self->cost_[0], sizeof(float) * cost_size, status);
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
void MulticlassPerceptronModel::Save(const char *filename_prefix,
                                     Status *status) {
  std::string prefix = filename_prefix;
  std::string metafile = prefix + ".meta";
  std::string xindex_file = prefix + ".x.idx";
  std::string cost_file = prefix + ".cost.data";

  // Metadata file
  WritableFile *fd = WritableFile::New(metafile.c_str(), status);
  if (status->ok()) {
    int32_t magic_number = kMulticlassPerceptronModelMagicNumber;
    fd->WriteValue<int32_t>(magic_number, status);
  }

  if (status->ok()) fd->WriteValue<int32_t>(xsize_, status);
  if (status->ok()) fd->WriteValue<int32_t>(ysize(), status);
  if (status->ok()) fd->WriteValue<int32_t>(xindex_->size(), status);
  if (status->ok()) {
    // Converts std::vector<std::string> into char (*)[kLabelSizeMax] and writes
    // to `fd`
    char *buff = new char[ysize() * kLabelSizeMax];
    char (*yname_buf)[kLabelSizeMax] = 
        reinterpret_cast<char (*)[kLabelSizeMax]>(buff);
    for (int yid = 0; yid < ysize(); ++yid) {
      strlcpy(yname_buf[yid], yname_[yid].c_str(), kLabelSizeMax);
    }
    fd->Write(yname_buf, ysize() * kLabelSizeMax, status);
    delete buff;
  }
  delete fd;
  
  // Index file for x (ReimuTrie)
  if (status->ok()) {
    if (xindex_->Save(xindex_file.c_str()) == false) {
      *status = Status::IOError(xindex_file.c_str());
    }
  }

  // Cost data file
  if (status->ok()) fd = WritableFile::New(cost_file.c_str(), status);
  if (status->ok()) {
    int cost_size = sizeof(float) * xsize_ * ysize();
    fd->Write(&cost_[0], cost_size, status);
  }
  delete fd;
}

MulticlassPerceptronModel::~MulticlassPerceptronModel() {
  delete xindex_;
  xindex_ = NULL;

  delete yindex_;
  yindex_ = NULL;
}

int MulticlassPerceptronModel::GetOrInsertXId(const char *xname) {
  int val;
  if (xindex_->Get(xname, &val)) {
    return val;
  } else {
    xindex_->Put(xname, xsize_);
    ++xsize_;
    cost_.resize(ysize() * xsize_);
    return xsize_ - 1;
  }
}

int MulticlassPerceptronModel::yid(const char *yname) const { 
  int val;
  if (yindex_->Get(yname, &val)) {
    return val;
  } else {
    return kIdNone;
  }
}

int MulticlassPerceptronModel::xid(const char *xname) const {
  int val;
  if (xindex_->Get(xname, &val)) {
    return val;
  } else {
    return kIdNone;
  }
}

}  // namespace milkcat
