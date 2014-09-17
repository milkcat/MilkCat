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
// dependency_node.h --- Created at 2014-09-16
//

#ifndef SRC_PARSER_DEPENDENCY_NODE_H_
#define SRC_PARSER_DEPENDENCY_NODE_H_

namespace milkcat {

class DependencyParser;

class DependencyParser::Node {
 public:
  static const int kNone = -1;

  // Reset to initial values
  void Initialize(int id) {
    id_ = id;
    head_id_ = kNone;
    right_child_id_ = kNone;
    left_child_id_ = kNone;
    strcpy(dependency_label_, "NULL");
  }

  int id() const { return id_; }

  int head_id() const { return head_id_; }
  void set_head_id(int head_id) { head_id_ = head_id; }

  const char *dependency_label() const { return dependency_label_; }
  void set_dependency_label(const char *label) {
    strncpy(dependency_label_, label, sizeof(dependency_label_) - 1);
  }
  
  // Get the id of left and right child
  int left_child_id() const { return left_child_id_; }
  int right_child_id() const { return right_child_id_; }
  
  // Add a child to this node
  void AddChild(int child_id) {
    if (child_id < id_) {
      if (child_id < left_child_id_ || left_child_id_ == kNone)
        left_child_id_ = child_id;
    } else {
      if (child_id > right_child_id_ || right_child_id_ == kNone)
        right_child_id_ = child_id;
    }
  }

 private:
  int id_;
  int head_id_;
  int left_child_id_;
  int right_child_id_;
  char dependency_label_[10];
};

}  // namespace milkcat

#endif  // SRC_PARSER_DEPENDENCY_NODE_H_