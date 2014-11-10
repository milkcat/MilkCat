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
// dependency_state.h --- Created at 2014-09-03
// state.h --- Created at 2014-10-27
//

#ifndef SRC_PARSER_STATE_H_
#define SRC_PARSER_STATE_H_

#include <vector>
#include "parser/dependency_parser.h"
#include "utils/utils.h"

namespace milkcat {

template<class T> class Pool;

// State in the dependency parser, including buffer, stack, tree ...
// Implemented the unshift transition in (Nivre, 14) Arc-Eager Parsing with the
// Tree Constraint
class DependencyParser::State {
 public:
  enum {
    kMaxStackSize = 128,
    kMaxInputSize = 1024
  };

  State();

  // Initialize and allocate nodes for the sentence
  void Initialize(Pool<Node> *node_pool, int sentance_length);

  // Transitions
  void Shift();
  void Unshift();
  void Reduce();
  void LeftArc(const char *label);
  void RightArc(const char *label);

  // Indicates whether current status allows these transitions
  bool AllowShift() const;
  bool AllowReduce() const;
  bool AllowLeftArc() const;
  bool AllowRightArc(bool is_root) const;

  // Get node from stack or input
  const Node *Stack(int idx) const;
  const Node *Input(int idx) const;

  // Relative nodes of a node
  const Node *Parent(const Node *node) const;
  const Node *LeftChild(const Node *node) const;
  const Node *RightChild(const Node *node) const;

  // Input and stack status
  bool InputEnd() const { return input_top_ == 0; }
  bool StackEmpty() const { return stack_top_ == 0; }
  bool StackOnlyOneElement() const { return stack_top_ == 1; }
  bool StackFull() const { return stack_top_ == kMaxStackSize - 1; }

  // The n-th node in the input buffer
  const Node *node_at(int n) { return input_[n]; }

  int input_size() const { return input_size_; }
  int stack_top() const { return stack_top_; }

  // The functions below are only used in `BeamArceagerDependencyParser`

  // Copy current state to `target_state`
  void CopyTo(State *target_state) const;

  double weight() const { return weight_; }
  void set_weight(double weight) { weight_ = weight; }

  // The previous state that before a transition
  State *previous() const { return previous_; }
  void set_previous(State *previous) { previous_ = previous; }

  bool correct() const { return correct_; }
  void set_correct(bool correct) { correct_ = correct; }

  int last_transition() const { return last_transition_; }
  void set_last_transition(int last_transition) { 
    last_transition_ = last_transition;
  }

 private:
  Pool<Node> *node_pool_;
  
  // Stores the index of node in input_
  int stack_[kMaxStackSize];
  int input_stack_[kMaxInputSize];
  
  Node *input_[kMaxInputSize];
  int stack_top_;
  int input_size_;
  int input_top_;

  bool end_reached_;
  double weight_;

  State *previous_;
  bool correct_;
  int last_transition_;
  bool have_root_;
  DISALLOW_COPY_AND_ASSIGN(State);
};

}  // namespace milkcat

#endif  // SRC_PARSER_STATE_H_