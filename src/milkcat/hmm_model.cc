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

// Use the second order Hidden Markov Model (TnT Model) for tagging
// see also:
//     TnT -- A Statistical Part-of-Speech Tagger
//     http://aclweb.org/anthology//A/A00/A00-1031.pdf

// HMMModel data file struct
// -------------------------
// int32_t kHmmModelMagicNumber
// int32_t tag_num
// int32_t max_termid
// int32_t emit_num
// char[kTagStrLenMax * tag_num] tag_str
// float[tag_num] tag_cost
// float[tag_num ^ 3] transition_matrix
// HMMEmitRecord[emit_num] emits

#include "milkcat/hmm_model.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <map>
#include <string>
#include "common/milkcat_config.h"
#include "common/trie_tree.h"
#include "utils/utils.h"
#include "utils/readable_file.h"
#include "utils/writable_file.h"

namespace milkcat {

#pragma pack(1)
struct HMMEmitRecord {
  int32_t term_id;
  int32_t tag_id;
  float cost;
};
#pragma pack(0)

HMMModel *HMMModel::New(const char *model_path, Status *status) {
  HMMModel *self = new HMMModel();
  ReadableFile *fd = ReadableFile::New(model_path, status);

  int32_t magic_number = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&magic_number, status);

  LOG_IF(status->ok(), "Magic number is %d\n", magic_number);

  if (magic_number != kHmmModelMagicNumber) 
    *status = Status::Corruption(model_path);

  if (status->ok()) fd->ReadValue<int32_t>(&(self->tag_num_), status);
  if (status->ok()) fd->ReadValue<int32_t>(&(self->max_term_id_), status);

  int tag_num = self->tag_num_;
  int32_t emit_num = 0;
  if (status->ok()) fd->ReadValue<int32_t>(&emit_num, status);

  LOG_IF(status->ok(), "Tag number is %d\n", tag_num);
  LOG_IF(status->ok(), "Emit number is %d\n", emit_num);

  self->tag_str_ = reinterpret_cast<char (*)[kTagStrLenMax]>(
      new char[kTagStrLenMax * tag_num]);
  for (int i = 0; i < tag_num && status->ok(); ++i) {
    fd->Read(self->tag_str_[i], 16, status);
  }

  if (status->ok()) {
    self->tag_cost_ = new float[tag_num];
    fd->Read(self->tag_cost_, sizeof(float) * tag_num, status);
  }
  
  self->transition_matrix_ = new float[tag_num * tag_num * tag_num];
  float f_weight;
  for (int i = 0; i < tag_num * tag_num * tag_num && status->ok(); ++i) {
    fd->ReadValue<float>(&f_weight, status);
    self->transition_matrix_[i] = f_weight;
  }

  self->emits_ = new Emit *[self->max_term_id_ + 1];
  memset(self->emits_, 0, sizeof(Emit *) * (self->max_term_id_ + 1));
  HMMEmitRecord emit_record;
  Emit *emit_node;
  for (int i = 0; i < emit_num && status->ok(); ++i) {
    fd->ReadValue<HMMEmitRecord>(&emit_record, status);
    emit_node = new Emit(emit_record.tag_id, 
                         emit_record.cost, 
                         self->emits_[emit_record.term_id]);
    self->emits_[emit_record.term_id] = emit_node;
  }

  LOG_IF(status->ok(), "File size: %ld, Tell: %ld\n", fd->Size(), fd->Tell());

  if (status->ok() && fd->Tell() != fd->Size())
    *status = Status::Corruption(model_path);

  delete fd;
  if (!status->ok()) {
    delete self;
    return NULL;
  } else {
    return self;
  }
}

void HMMModel::LoadYTagFromText(HMMModel *self,
                                const char *yset_model_path,
                                std::map<std::string, int> *y_tag,
                                Status *status) {
  char buff[1024];  
  char tagstr[1024] = "\0";
  double cost = 0;

  std::vector<float> tag_cost;
  ReadableFile *fd = ReadableFile::New(yset_model_path, status);
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(buff, sizeof(buff), status);
    if (status->ok()) {
      // Remove the LF character
      sscanf(buff, "%s %lf", tagstr, &cost);
      y_tag->insert(std::pair<std::string, int>(tagstr, y_tag->size()));
      tag_cost.push_back(static_cast<float>(cost));
    }
  }
  delete fd;

  if (status->ok()) {
    self->tag_str_ = reinterpret_cast<char (*)[16]>(
        new char[kTagStrLenMax * y_tag->size()]);
    std::map<std::string, int>::iterator it;
    for (it = y_tag->begin(); it != y_tag->end(); ++it) {
      utils::strlcpy(self->tag_str_[it->second],
                     it->first.c_str(),
                     kTagStrLenMax);
    }
    self->tag_num_ = y_tag->size();

    self->tag_cost_ = new float[self->tag_num_];
    for (int i = 0; i < self->tag_num_; ++i)
      self->tag_cost_[i] = tag_cost[i];
  }
}

void HMMModel::LoadTransFromText(
    HMMModel *self,
    const char *trans_model_path,
    const std::map<std::string, int> &y_tag,
    Status *status) {
  char buff[1024];
  char leftleft_tagstr[1024] = "\0",
       left_tagstr[1024] = "\0",
       tagstr[1024] = "\0";
  double cost = 0;

  int trans_matrix_size = self->tag_num_ * self->tag_num_ * self->tag_num_;
  self->transition_matrix_ = new float[trans_matrix_size];  

  // Get the transition data from trans_model file
  ReadableFile *fd = NULL;
  std::map<std::string, int>::const_iterator it_leftleft, it_left, it_curr;
  if (status->ok()) fd = ReadableFile::New(trans_model_path, status);
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(buff, sizeof(buff), status);
    if (status->ok()) {
      sscanf(buff, 
             "%s %s %s %lf",
             leftleft_tagstr,
             left_tagstr,
             tagstr,
             &cost);
      it_leftleft = y_tag.find(leftleft_tagstr);
      it_left = y_tag.find(left_tagstr);
      it_curr = y_tag.find(tagstr);

      // If some tag is not in tag set
      if (it_curr == y_tag.end() || 
          it_left == y_tag.end() || 
          it_leftleft == y_tag.end()) {
        sprintf(buff, 
                "%s: Invalid tag trigram %s %s %s "
                "(one of them is not in tag set).", 
                trans_model_path,
                leftleft_tagstr,
                left_tagstr,
                tagstr);
        *status = Status::Corruption(buff);
      }
    }
    
    if (status->ok()) {
      int leftleft_tag = it_leftleft->second;
      int left_tag = it_left->second;
      int curr_tag = it_curr->second;
      self->transition_matrix_[leftleft_tag * self->tag_num_ * self->tag_num_ +
                               left_tag * self->tag_num_ +
                               curr_tag] = static_cast<float>(cost);
    }
  }
  delete fd;
}

void HMMModel::LoadEmitFromText(
    HMMModel *self,
    const char *emit_model_path,
    const char *index_path,
    const std::map<std::string, int> &y_tag,
    Status *status) {
  char buff[1024];
  char tagstr[1024] = "\0";
  double cost = 0;

  // Get emit data from file
  ReadableFile *fd = NULL;
  TrieTree *index = NULL;
  std::map<int, Emit *> emit_map;
  char word[1024] = "\0";
  if (status->ok()) index = DoubleArrayTrieTree::New(index_path, status);
  if (status->ok()) fd = ReadableFile::New(emit_model_path, status);

  self->max_term_id_ = 0;
  self->emit_num_ = 0;
  int term_id;
  std::map<std::string, int>::const_iterator it_curr;
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(buff, sizeof(buff), status);
    if (status->ok()) {
      sscanf(buff, 
             "%s %s %lf",
             tagstr,
             word,
             &cost);
      term_id = index->Search(word);
      if (term_id > self->max_term_id_) self->max_term_id_ = term_id;
      if (term_id < 0) continue;

      it_curr = y_tag.find(tagstr);
      if (it_curr == y_tag.end()) {
        sprintf(buff, 
                "%s: Invalid tag %s (not in tag set).", 
                emit_model_path,
                tagstr);
        *status = Status::Corruption(buff);
      }
    }

    if (status->ok()) {
      std::map<int, Emit *>::iterator it_emit = emit_map.find(term_id);
      if (it_emit == emit_map.end()) {
        emit_map[term_id] = new Emit(it_curr->second, 
                                     static_cast<float>(cost),
                                     NULL);
      } else {
        Emit *next = it_emit->second;
        emit_map[term_id] = new Emit(it_curr->second, 
                                     static_cast<float>(cost),
                                     next);
      }
    }

    self->emit_num_++;
  }
  delete fd;
  delete index;

  if (status->ok()) {
    self->emits_ = new Emit *[self->max_term_id_ + 1];
    for (int i = 0; i <= self->max_term_id_; ++i) self->emits_[i] = NULL;

    std::map<int, Emit *>::iterator it;
    for (it = emit_map.begin(); it != emit_map.end(); ++it) {
      self->emits_[it->first] = it->second;
    }
  }
}

// Create the HMMModel instance from some text model file
HMMModel *HMMModel::NewFromText(const char *trans_model_path, 
                                const char *emit_model_path,
                                const char *yset_model_path,
                                const char *index_path,
                                Status *status) {
  HMMModel *self = new HMMModel();
  std::map<std::string, int> y_tag;
  LoadYTagFromText(self, yset_model_path, &y_tag, status);

  if (status->ok())
    LoadTransFromText(self, trans_model_path, y_tag, status);
  if (status->ok())
    LoadEmitFromText(self, emit_model_path, index_path, y_tag, status);

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

void HMMModel::Save(const char *model_path, Status *status) {
  WritableFile *fd = WritableFile::New(model_path, status);

  if (status->ok()) fd->WriteValue<int32_t>(kHmmModelMagicNumber, status);
  if (status->ok()) fd->WriteValue<int32_t>(tag_num_, status);
  if (status->ok()) fd->WriteValue<int32_t>(max_term_id_, status);
  if (status->ok()) fd->WriteValue<int32_t>(emit_num_, status);

  if (status->ok()) fd->Write(tag_str_, kTagStrLenMax * tag_num_, status);
  if (status->ok()) fd->Write(tag_cost_, sizeof(float) * tag_num_, status);
  if (status->ok()) {
    fd->Write(transition_matrix_, 
              sizeof(float) * tag_num_ * tag_num_ * tag_num_,
              status);
  }

  HMMEmitRecord emit_record;
  Emit *emit = NULL;

  int emit_num = 0;
  for (int term_id = 0; status->ok() && term_id <= max_term_id_; ++term_id) {
    emit = emits_[term_id];
    while (status->ok() && emit) {
      emit_record.term_id = term_id;
      emit_record.tag_id = emit->tag;
      emit_record.cost = emit->cost;
      fd->WriteValue<HMMEmitRecord>(emit_record, status);

      emit = emit->next;
      emit_num++;
    }
  }

  LOG_IF(status->ok(), "%d emit record writted\n", emit_num);

  delete fd;
}


HMMModel::HMMModel(): emits_(NULL),
                      max_term_id_(0),
                      emit_num_(0),
                      tag_num_(0),
                      tag_str_(NULL),
                      transition_matrix_(NULL),
                      tag_cost_(NULL) {
}

HMMModel::~HMMModel() {
  delete[] transition_matrix_;
  transition_matrix_ = NULL;

  delete[] tag_str_;
  tag_str_ = NULL;

  delete[] tag_cost_;
  tag_cost_ = NULL;

  Emit *p, *q;
  if (emits_ != NULL) {
    for (int i = 0; i < max_term_id_ + 1; ++i) {
      p = emits_[i];
      while (p) {
        q = p->next;
        delete p;
        p = q;
      }
    }
  }
  delete[] emits_;
}

}  // namespace milkcat
