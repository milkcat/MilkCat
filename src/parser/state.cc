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
// dependency_state.cc --- Created at 2014-09-15
// state.cc --- Created at 2014-10-27
//

#include "parser/state.h"

#include <algorithm>
#include "parser/node.h"
#include "util/pool.h"

namespace milkcat {

DependencyParser::State::State(): sentence_length_(0),
                                  previous_(NULL),
                                  correct_(true),
                                  last_transition_(0),
                                  have_root_(false) {
}

void DependencyParser::State::Initialize(Pool<Node> *node_pool,
                                         int sentance_length) {
  sentence_length_ = std::min(sentance_length + 1,
                              static_cast<int>(kMaxInputSize));

  node_pool_ = node_pool;
  for (int i = 0; i < sentance_length + 1; ++i) {
    Node *node = node_pool->Alloc();
    node->Initialize(i);
    sentence_[i] = node;
  }

  input_.clear();
  for (int nodeid = sentance_length; nodeid >= 1; --nodeid) {
    input_.push_back(nodeid);
  }

  // Push the root node
  stack_.clear();
  stack_.push_back(0);

  // Root node has already pushed to the stack, `input_stack[sentance_length]`
  // is the root node
  weight_ = 0.0;
  previous_ = NULL;
  correct_ = true;
  last_transition_ = -1;
  have_root_ = false;
}

void DependencyParser::State::Shift() {
  MC_ASSERT(!InputEnd(), "out of bound");
  stack_.push_back(input_.back());
  input_.pop_back();
}

void DependencyParser::State::LeftArc(const char *label) {
  MC_ASSERT(!StackEmpty(), "stack empty");
  MC_ASSERT(!InputEnd(), "out of bound");

  Node *stack0 = sentence_[stack_.back()];
  Node *input0 = sentence_[input_.back()];

  stack0->set_head_id(input0->id());
  stack0->set_dependency_label(label);
  stack_.pop_back();

  input0->AddChild(stack0->id());
}

void DependencyParser::State::RightArc(const char *label) {
  MC_ASSERT(!InputEnd(), "out of bound");
  Node *stack0 = sentence_[stack_.back()];
  Node *input0 = sentence_[input_.back()];

  input0->set_head_id(stack0->id());
  input0->set_dependency_label(label);
  input_.pop_back();

  stack0->AddChild(input0->id());
  stack_.pop_back();
  input_.push_back(stack0->id());
}

bool DependencyParser::State::AllowShift() const {
  if (input_.size() == 1 && StackEmpty() == false) return false;
  if (InputEnd()) return false;
  return true;
}

bool DependencyParser::State::AllowLeftArc() const {
  if (StackEmpty()) return false;
  if (InputEnd()) return false;
  if (sentence_[stack_.back()]->id() == 0) return false;
  if (sentence_[stack_.back()]->head_id() != Node::kNone) return false;
  return true;
}

bool DependencyParser::State::AllowRightArc(bool is_root) const {
  if (StackEmpty()) return false;
  if (InputEnd()) return false;
  if (is_root == true && have_root_ == true) return false;
  if (is_root == true && StackOnlyOneElement() == false) return false;
  if (is_root == false && StackOnlyOneElement() == true) return false;
  if (sentence_[input_.back()]->head_id() != Node::kNone) {
    return false;
  }
  return true;
}

const DependencyParser::Node *DependencyParser::State::Stack(int idx) const {
  int stack_idx = stack_.size() - idx - 1;
  if (stack_idx >= 0) {
    return sentence_[stack_[stack_idx]];
  } else {
    return NULL;
  }
}

const DependencyParser::Node *DependencyParser::State::Input(int idx) const {
  int input_idx = input_.size() - idx - 1;
  if (input_idx >= 0) 
    return sentence_[input_[input_idx]];
  else
    return NULL;
}

const DependencyParser::Node *
DependencyParser::State::Parent(const Node *node) const {
  if (node == NULL) return NULL;
  int head_id = node->head_id();
  if (head_id == Node::kNone) return NULL;
  return sentence_[head_id];
}

const DependencyParser::Node *
DependencyParser::State::LeftChild(const Node *node) const {
  if (node == NULL) return NULL;
  int left_child_id = node->left_child_id();
  if (left_child_id == Node::kNone) return NULL;
  return sentence_[left_child_id];
}

const DependencyParser::Node *
DependencyParser::State::RightChild(const Node *node) const {
  if (node == NULL) return NULL;
  int right_child_id = node->right_child_id();
  if (right_child_id == Node::kNone) return NULL;
  return sentence_[right_child_id];
}

void DependencyParser::State::CopyTo(State *target_state) const {
  target_state->node_pool_ = node_pool_;
  target_state->sentence_length_ = sentence_length_;
  target_state->weight_ = weight_;
  target_state->correct_ = correct_;
  target_state->previous_ = previous_;
  target_state->last_transition_ = last_transition_;

  for (int i = 0; i < sentence_length_; ++i) {
    Node *node = node_pool_->Alloc();
    sentence_[i]->CopyTo(node);
    target_state->sentence_[i] = node;
  }

  target_state->stack_ = stack_;
  target_state->input_ = input_;
}

}  // namespace milkcat
