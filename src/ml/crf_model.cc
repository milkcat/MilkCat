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
// crfpp_model.cc --- Created at 2013-10-28
// crf_model.cc --- Created at 2013-11-02
//

#include "ml/crf_model.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "common/milkcat_config.h"
#include "common/reimu_trie.h"
#include "util/readable_file.h"
#include "util/util.h"
#include "util/writable_file.h"

namespace milkcat {

CRFModel::CRFModel(): xindex_(NULL),
                      unigram_cost_(NULL),
                      bigram_cost_(0),
                      bigram_xsize_(0), 
                      unigram_xsize_(0) {
}

CRFModel::~CRFModel() {
  delete xindex_;
  xindex_ = NULL;

  delete unigram_cost_;
  unigram_cost_ = NULL;

  delete bigram_cost_;
  bigram_cost_ = NULL;
}

int CRFModel::xid(const char *xname) const {
  return xindex_->Get(xname, -1);
}

CRFModel *CRFModel::OpenText(const char *text_filename,
                             const char *template_filename,
                             Status *status) {
  char line[4096],
       xname[4096],
       left_yname[4096],
       right_yname[4096];
  float cost;

  // Gets all y in text file
  ReadableFile *fd = ReadableFile::New(text_filename, status);
  std::set<std::string> yname_set;
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      trim(line);
      sscanf(line, "%s\t%s\t%s\t%f", xname, left_yname, right_yname, &cost);
      if (line[0] == 'b') yname_set.insert(left_yname);
      yname_set.insert(right_yname);
    }
  }
  delete fd;
  fd = NULL;

  // Read data from text file
  std::map<std::string, int> yname_idx;
  CRFModel *self = NULL;
  int ysize = 0;
  if (status->ok()) {
    self = new CRFModel();
    self->xindex_ = new ReimuTrie();
    self->y_ = std::vector<std::string>(yname_set.begin(), yname_set.end());
    ysize = self->y_.size();
    for (int idx = 0; idx < self->y_.size(); ++idx) {
      yname_idx[self->y_[idx]] = idx;
    }
    fd = ReadableFile::New(text_filename, status);
  }
  std::vector<float> unigram_cost,
                     bigram_cost;
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      trim(line);
      sscanf(line, "%s\t%s\t%s\t%f", xname, left_yname, right_yname, &cost);
      if (xname[0] == 'b') {
        int xid = self->xindex_->Get(xname, -1);
        if (xid < 0) {
          self->xindex_->Put(xname, self->bigram_xsize_);
          xid = self->bigram_xsize_;
          ++self->bigram_xsize_;
          bigram_cost.resize(self->bigram_xsize_ * ysize * ysize);
        }
        int left_yid = yname_idx[left_yname];
        int right_yid = yname_idx[right_yname];
        int idx = xid * ysize * ysize + left_yid * ysize + right_yid;
        bigram_cost[idx] = cost;
      } else if (xname[0] == 'u') {
        int xid = self->xindex_->Get(xname, -1);
        if (xid < 0) {
          self->xindex_->Put(xname, self->unigram_xsize_);
          xid = self->unigram_xsize_;
          ++self->unigram_xsize_;
          unigram_cost.resize(self->unigram_xsize_ * ysize);
        }
        int right_yid = yname_idx[right_yname];
        int idx = xid * ysize + right_yid;
        unigram_cost[idx] = cost;
      } else {
        *status = Status::Corruption(text_filename);
      }
    }
  }
  if (status->ok()) {
    self->unigram_cost_ = StaticArray<float>::NewFromArray(
        &unigram_cost[0], unigram_cost.size());
    self->bigram_cost_ = StaticArray<float>::NewFromArray(
        &bigram_cost[0], bigram_cost.size());
  }
  delete fd;
  fd = NULL;

  // Reads templates
  if (status->ok()) {
    fd = ReadableFile::New(template_filename, status);
  }
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      trim(line);
      if (line[0] == 'b') {
        self->bigram_tmpl_.push_back(line);
      } else if (line[0] == 'u') {
        self->unigram_tmpl_.push_back(line);
      } else {
        *status = Status::Corruption(template_filename);
      }
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

void CRFModel::Save(const char *model_prefix, Status *status) {
  std::string prefix = model_prefix;
  std::string xindex_filename = prefix + ".x.idx";
  std::string bigram_cost_filename = prefix + ".cost.bi";
  std::string unigram_cost_filename = prefix + ".cost.uni";
  std::string meta_filename = prefix + ".meta";

  int success = xindex_->Save(xindex_filename.c_str());
  if (!success) *status = Status::IOError(xindex_filename.c_str());

  if (status->ok()) {
    unigram_cost_->Save(unigram_cost_filename.c_str(), status);
  }
  if (status->ok()) {
    bigram_cost_->Save(bigram_cost_filename.c_str(), status);
  }

  WritableFile *fd = NULL;
  if (status->ok()) {
    fd = WritableFile::New(meta_filename.c_str(), status);
  }
  
  char buffer[kFeatureLengthMax];
  if (status->ok()) fd->WriteValue<int32_t>(kCrfModelMagicNumber, status);
  if (status->ok()) fd->WriteValue<int32_t>(y_.size(), status);
  if (status->ok()) {
    fd->WriteValue<int32_t>(unigram_tmpl_.size() + bigram_tmpl_.size(),
                            status);
  }
  if (status->ok()) fd->WriteValue<int32_t>(unigram_xsize_, status);
  if (status->ok()) fd->WriteValue<int32_t>(bigram_xsize_, status);

  // Writes part-of-speech tags
  for (std::vector<std::string>::iterator
       it = y_.begin(); status->ok() && it != y_.end(); ++it) {
    strlcpy(buffer, it->c_str(), kPOSTagLengthMax);
    fd->Write(buffer, kPOSTagLengthMax, status);
  }

  // Writes templates
  for (std::vector<std::string>::iterator
       it = unigram_tmpl_.begin(); 
       status->ok() && it != unigram_tmpl_.end(); 
       ++it) {
    strlcpy(buffer, it->c_str(), kFeatureLengthMax);
    fd->Write(buffer, kFeatureLengthMax, status);
  }
  for (std::vector<std::string>::iterator
       it = bigram_tmpl_.begin(); 
       status->ok() && it != bigram_tmpl_.end(); 
       ++it) {
    strlcpy(buffer, it->c_str(), kFeatureLengthMax);
    fd->Write(buffer, kFeatureLengthMax, status);
  }

  delete fd;
}

CRFModel *CRFModel::New(const char *model_prefix, Status *status) {
  std::string prefix = model_prefix;
  std::string xindex_filename = prefix + ".x.idx";
  std::string bigram_cost_filename = prefix + ".cost.bi";
  std::string unigram_cost_filename = prefix + ".cost.uni";
  std::string meta_filename = prefix + ".meta";

  CRFModel *self = new CRFModel();

  self->xindex_ = ReimuTrie::Open(xindex_filename.c_str());
  if (self->xindex_ == NULL) *status = Status::IOError(xindex_filename.c_str());

  if (status->ok()) {
    self->unigram_cost_ = StaticArray<float>::New(
        unigram_cost_filename.c_str(), status);
  }
  if (status->ok()) {
    self->bigram_cost_ = StaticArray<float>::New(
        bigram_cost_filename.c_str(), status);
  }

  ReadableFile *fd = NULL;
  if (status->ok()) {
    fd = ReadableFile::New(meta_filename.c_str(), status);
  }
  int32_t magic_number;
  if (status->ok()) {
    fd->ReadValue<int32_t>(&magic_number, status);
  }
  if (status->ok() && magic_number != kCrfModelMagicNumber) {
    *status = Status::Corruption(meta_filename.c_str());
  }
  int32_t ysize = 0,
          tmpl_size = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&ysize, status);
  if (status->ok()) fd->ReadValue<int32_t>(&tmpl_size, status);
  if (status->ok()) {
    fd->ReadValue<int32_t>(&(self->unigram_xsize_), status);
  }
  if (status->ok()) {
    fd->ReadValue<int32_t>(&(self->bigram_xsize_), status);
  }
  char buffer[kFeatureLengthMax];
  for (int yid = 0; status->ok() && yid < ysize; ++yid) {
    fd->Read(buffer, kPOSTagLengthMax, status);
    if (status->ok()) self->y_.push_back(buffer);
  }
  for (int tmpl = 0; status->ok() && tmpl < tmpl_size; ++tmpl) {
    fd->Read(buffer, kFeatureLengthMax, status);
    if (status->ok()) {
      if (buffer[0] == 'u') {
        self->unigram_tmpl_.push_back(buffer);
      } else if (buffer[0] == 'b') {
        self->bigram_tmpl_.push_back(buffer);
      } else {
        *status = Status::Corruption(meta_filename.c_str());
      }
    }
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

}  // namespace milkcat
