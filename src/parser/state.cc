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
#include "utils/pool.h"

namespace milkcat {

DependencyParser::State::State(): stack_top_(0),
                                  input_size_(0),
                                  input_top_(0),
                                  end_reached_(false) {
}

void DependencyParser::State::Initialize(Pool<Node> *node_pool,
                                         int sentance_length) {
  input_size_ = std::min(sentance_length + 1,
                         static_cast<int>(kMaxInputSize));

  node_pool_ = node_pool;
  for (int i = 0; i < sentance_length + 1; ++i) {
    Node *node = node_pool->Alloc();
    node->Initialize(i);
    input_[i] = node;
    input_stack_[sentance_length - i] = i;
  }

  // Push the root node
  stack_[0] = 0;

  // Root node has already pushed to the stack, `input_stack[sentance_length]`
  // is the root node
  input_top_ = sentance_length;
  stack_top_ = 1;
  end_reached_ = false;
  weight_ = 0.0;
}

void DependencyParser::State::Shift() {
  ASSERT(!InputEnd(), "Out of bound");
  ASSERT(!StackFull(), "Stack overflow");

  stack_[stack_top_] = input_stack_[input_top_ - 1];
  input_top_--;
  stack_top_++;

  if (InputEnd()) end_reached_ = true;
}

void DependencyParser::State::Unshift() {
  ASSERT(!StackOnlyOneElement(), "Stack empty");
  input_stack_[input_top_] = stack_[stack_top_ - 1];
  input_top_++;
  stack_top_--;
}

void DependencyParser::State::Reduce() {
  ASSERT(!StackEmpty(), "Stack empty");

  stack_top_--;
}

void DependencyParser::State::LeftArc(const char *label) {
  ASSERT(!StackEmpty(), "Stack empty");
  ASSERT(!InputEnd(), "Out of bound");

  Node *stack0 = input_[stack_[stack_top_ - 1]];
  Node *input0 = input_[input_stack_[input_top_ - 1]];

  stack0->set_head_id(input0->id());
  stack0->set_dependency_label(label);
  stack_top_--;

  input0->AddChild(stack0->id());
}

void DependencyParser::State::RightArc(const char *label) {
  ASSERT(!InputEnd(), "Out of bound");
  ASSERT(!StackFull(), "Stack overflow");

  Node *stack0 = input_[stack_[stack_top_ - 1]];
  Node *input0 = input_[input_stack_[input_top_ - 1]];

  input0->set_head_id(stack0->id());
  input0->set_dependency_label(label);
  input_top_--;

  stack0->AddChild(input0->id());
  stack_[stack_top_] = input0->id();
  stack_top_++;

  if (InputEnd()) end_reached_ = true;
}

bool DependencyParser::State::AllowShift() const {
  if (StackFull()) return false;
  if (InputEnd()) return false;
  if (input_top_ == 0) return false;
  if (end_reached_) return false;
  return true;
}

bool DependencyParser::State::AllowReduce() const {
  if (StackEmpty()) return false;
  if (input_[stack_[stack_top_ - 1]]->id() == 0) return false;
  if (input_[stack_[stack_top_ - 1]]->head_id() == Node::kNone) return false;
  return true;
}

bool DependencyParser::State::AllowLeftArc() const {
  if (StackEmpty()) return false;
  if (InputEnd()) return false;
  if (input_[stack_[stack_top_ - 1]]->id() == 0) return false;
  if (input_[stack_[stack_top_ - 1]]->head_id() != Node::kNone) return false;
  return true;
}

bool DependencyParser::State::AllowRightArc() const {
  if (StackFull()) return false;
  if (InputEnd()) return false;
  return true;
}

const DependencyParser::Node *DependencyParser::State::Stack(int idx) const {
  int stack_top = stack_top_ - idx;
  if (stack_top > 0)
    return input_[stack_[stack_top - 1]];
  else
    return NULL;
}

const DependencyParser::Node *DependencyParser::State::Input(int idx) const {
  int input_top = input_top_ - idx;
  if (input_top > 0) 
    return input_[input_stack_[input_top - 1]];
  else
    return NULL;
}

const DependencyParser::Node *
DependencyParser::State::Parent(const Node *node) const {
  if (node == NULL) return NULL;
  int head_id = node->head_id();
  if (head_id == Node::kNone) return NULL;
  return input_[head_id];
}

const DependencyParser::Node *
DependencyParser::State::LeftChild(const Node *node) const {
  if (node == NULL) return NULL;
  int left_child_id = node->left_child_id();
  if (left_child_id == Node::kNone) return NULL;
  return input_[left_child_id];
}

const DependencyParser::Node *
DependencyParser::State::RightChild(const Node *node) const {
  if (node == NULL) return NULL;
  int right_child_id = node->right_child_id();
  if (right_child_id == Node::kNone) return NULL;
  return input_[right_child_id];
}

void DependencyParser::State::CopyTo(State *target_state) const {
  target_state->node_pool_ = node_pool_;
  target_state->stack_top_ = stack_top_;
  target_state->input_size_ = input_size_;
  target_state->input_top_ = input_top_;
  target_state->end_reached_ = end_reached_;
  target_state->weight_ = weight_;

  for (int i = 0; i < input_size_; ++i) {
    Node *node = node_pool_->Alloc();
    input_[i]->CopyTo(node);
    target_state->input_[i] = node;
  }

  for (int i = 0; i < stack_top_; ++i) {
    target_state->stack_[i] = stack_[i];
  }
  for (int i = 0; i < input_top_; ++i) {
    target_state->input_stack_[i] = input_stack_[i];
  }
}

}  // namespace milkcat
