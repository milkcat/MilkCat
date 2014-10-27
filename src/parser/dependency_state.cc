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
//

#define DEBUG

#include "parser/dependency_state.h"

#include <algorithm>
#include "parser/dependency_node.h"
#include "utils/log.h"
#include "utils/pool.h"

namespace milkcat {

DependencyParser::State::State(): stack_top_(0),
                                  input_size_(0),
                                  input_position_(0),
                                  cost_(0.0) {
}

void DependencyParser::State::Initialize(utils::Pool<Node> *node_pool,
                                         int sentance_length) {
  input_size_ = std::min(sentance_length + 1,
                         static_cast<int>(kMaxInputSize));


  for (int i = 0; i < sentance_length + 1; ++i) {
    Node *node = node_pool->Alloc();
    node->Initialize(i);
    input_[i] = node;
  }

  // Push the root node
  stack_[0] = input_[0];

  // Root node has already pushed to the stack
  input_position_ = 1;
  stack_top_ = 1;
}

void DependencyParser::State::Shift() {
  ASSERT(!InputEnd(), "Out of bound");
  ASSERT(!StackFull(), "Stack overflow");

  stack_[stack_top_] = input_[input_position_];
  input_position_++;
  stack_top_++;
}

void DependencyParser::State::Reduce() {
  ASSERT(!StackEmpty(), "Stack empty");

  stack_top_--;
}

void DependencyParser::State::LeftArc(const char *label) {
  ASSERT(!StackEmpty(), "Stack empty");

  Node *stack0 = stack_[stack_top_ - 1];
  Node *input0 = input_[input_position_];

  stack0->set_head_id(input0->id());
  stack0->set_dependency_label(label);
  stack_top_--;

  input0->AddChild(stack0->id());
}

void DependencyParser::State::RightArc(const char *label) {
  ASSERT(!InputEnd(), "Out of bound");
  ASSERT(!StackFull(), "Stack overflow");

  Node *stack0 = stack_[stack_top_ - 1];
  Node *input0 = input_[input_position_];

  input0->set_head_id(stack0->id());
  input0->set_dependency_label(label);
  input_position_++;

  stack0->AddChild(input0->id());
  stack_[stack_top_] = input0;
  stack_top_++;
}

bool DependencyParser::State::AllowShift() {
  if (StackFull()) return false;
  if (InputEnd()) return false;
  if (input_size_ - 1 == input_position_) return false;
  return true;
}

bool DependencyParser::State::AllowReduce() {
  if (StackEmpty()) return false;
  if (stack_[stack_top_ - 1]->id() == 0) return false;
  // if (stack_[stack_top_ - 1]->head_id() == Node::kNone) return false;
  return true;
}

bool DependencyParser::State::AllowLeftArc() {
  if (StackEmpty()) return false;
  if (InputEnd()) return false;
  if (stack_[stack_top_ - 1]->id() == 0) return false;
  if (stack_[stack_top_ - 1]->head_id() != Node::kNone) return false;
  return true;
}

bool DependencyParser::State::AllowRightArc() {
  if (StackFull()) return false;
  if (InputEnd()) return false;
  return true;
}

const DependencyParser::Node *DependencyParser::State::Stack(int idx) const {
  int stack_top = stack_top_ - idx;
  if (stack_top >= 0)
    return stack_[stack_top - 1];
  else
    return NULL;
}

const DependencyParser::Node *DependencyParser::State::Input(int idx) const {
  int input_position = input_position_ + idx;
  if (input_position < input_size_) 
    return input_[input_position];
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

}  // namespace milkcat
