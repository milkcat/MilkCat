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

#include "parser/tree_instance.h"
#include "util/string_builder.h"

namespace milkcat {

Orcale::Orcale(): instance_(NULL) {
}

void Orcale::Parse(const TreeInstance *instance) {
  instance_ = instance;

  stack_.clear();
  stack_.push_back(0);

  input_.clear();
  for (int nodeid = instance->size(); nodeid >= 1; --nodeid) {
    input_.push_back(nodeid);
  }
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

bool Orcale::IsRightArc() {
  // `stack_.back()` already have the head node
  if (stack_.size() == 0) return false;
  if (Head(input_.back()) != stack_.back()) return false;
  for (std::vector<int>::iterator
       it = input_.begin(); it != input_.end(); ++it) {
    if (Head(*it) == input_.back()) return false;
  }

  return true;
}

const char *Orcale::Next() {
  // OK, no more node in input
  if (input_.size() == 0) {
    if (stack_.size() == 1) return NULL;
    stack_.pop_back();
    return "reduce";
  } 

  StringBuilder string_builer(transition_label_, kLabelSizeMax);
  if (stack_.size() > 0 && Head(stack_.back()) == input_.back()) {
    string_builer << "leftarc_" << Label(stack_.back());
    stack_.pop_back();
    return transition_label_;
  } else if (IsRightArc()) {
    string_builer << "rightarc_" << Label(input_.back());
    input_.pop_back();
    input_.push_back(stack_.back());
    stack_.pop_back();
    return transition_label_;
  } else {
    stack_.push_back(input_.back());
    input_.pop_back();
    return "shift";
  }
}

}  // namespace milkcat
