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
// orcale.cc --- Created at 2014-10-21
//

#define DEBUG

#include "parser/orcale.h"

#include "parser/dependency_instance.h"
#include "utils/string_builder.h"

namespace milkcat {

Orcale::Orcale(): instance_(NULL),
                  input_ptr_(0) {
  string_builer_ = new StringBuilder();
}

Orcale::~Orcale() {
  delete string_builer_;
  string_builer_ = NULL;
}

void Orcale::Parse(const DependencyInstance *instance) {
  instance_ = instance;
  input_ptr_ = 0;

  stack_.clear();
  stack_.push_back(0);
}

inline int Orcale::Head(int nodeid) {
  // `0` is the root node
  if (nodeid == 0) return -1;
  return instance_->head_node_at(nodeid - 1);
}

inline const char *Orcale::Label(int nodeid) {
  // `0` is the root node
  if (nodeid == 0) return "ROOT";
  return instance_->dependency_type_at(nodeid - 1);
}

bool Orcale::IsReduce() {
  // `stack_.back()` already have the head node
  if (Head(stack_.back()) < stack_.back()) {
    int child = 0;
    for (int nodeid = input_ptr_ + 1; nodeid <= instance_->size(); ++nodeid) {
      if (Head(nodeid) == stack_.back()) ++child;
    }
    // if `stack_.back()` already have no child
    if (child == 0) return true;
  }

  return false;
}

const char *Orcale::Next() {
  // OK, no more node in input
  if (input_ptr_  == instance_->size()) {
    // Only root node lefts
    return NULL;
    /*
    if (stack_.size() == 1) return NULL;
    stack_.pop_back();
    return "reduce";
    */
  } 

  string_builer_->SetOutput(transition_label_, kLabelSizeMax);
  if (Head(stack_.back()) == input_ptr_ + 1) {
    *string_builer_ << "leftarc_" << Label(stack_.back());
    stack_.pop_back();
    return transition_label_;
  } else if (stack_.back() == Head(input_ptr_ + 1)) {
    *string_builer_ << "rightarc_" << Label(input_ptr_ + 1);
    stack_.push_back(input_ptr_ + 1);
    input_ptr_++;
    return transition_label_;
  } else if (IsReduce()) {
    stack_.pop_back();
    return "reduce";
  } else {
    stack_.push_back(input_ptr_ + 1);
    input_ptr_++;
    return "shift";
  }
}

}  // namespace milkcat
