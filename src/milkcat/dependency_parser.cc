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
// dependency_parser.h --- Created at 2013-08-10
//

#include "milkcat/dependency_parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include "common/maxent_classifier.h"
#include "milkcat/dependency_instance.h"
#include "milkcat/part_of_speech_tag_instance.h"
#include "milkcat/term_instance.h"
#include "utils/status.h"
#include "utils/string_builder.h"
#include "utils/utils.h"

namespace milkcat {


DependencyParser::DependencyParser(): maxent_classifier_(NULL),
                                      feature_buffer_(NULL) {
}

DependencyParser *DependencyParser::New(Model::Impl *model_impl,
                                        Status *status) {
  DependencyParser *self = new DependencyParser();
  const MaxentModel *maxent_model = model_impl->DependencyModel(status);

  if (status->ok()) {
    self->maxent_classifier_ = new MaxentClassifier(maxent_model);
    self->feature_buffer_ = new char *[kFeatures];
    for (int i = 0; i < kFeatures; ++i) {
      self->feature_buffer_[i] = new char[kFeatureStringMax];
    }
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

DependencyParser::~DependencyParser() {
  if (maxent_classifier_ != NULL) {  
    delete maxent_classifier_;
    maxent_classifier_ = NULL;
  }

  if (feature_buffer_ != NULL) {
    for (int i = 0; i < kFeatures; ++i)
      delete[] feature_buffer_[i];

    delete[] feature_buffer_; 
    feature_buffer_ = NULL;
  }
}

DependencyParser::Node *
DependencyParser::NodeFromStack(int top_index) const {
  // printf("%d %d\n", top_index, stack_.size());
  return top_index >= stack_.size()? NULL: 
                                     stack_[stack_.size() - 1 - top_index];
}

DependencyParser::Node *
DependencyParser::NodeFromBuffer(int index) const {
  int real_index = buffer_ptr_ + index;
  return real_index >= buffer_.size()? NULL: buffer_[real_index];
}

DependencyParser::Node *
DependencyParser::HeadNode(Node *node) const {
  int head_id = node->head_id();
  return head_id == Node::kNone? NULL: buffer_[head_id];
}

DependencyParser::Node *
DependencyParser::LeftmostDependentNode(Node *node) const {
  int ld = node->GetLeftmostDepententId();
  return ld == Node::kNone? NULL: buffer_[ld];
}

DependencyParser::Node *
DependencyParser::RightmostDependentNode(Node *node) const {
  int rd = node->GetRightmostDependentId();
  return rd == Node::kNone? NULL: buffer_[rd];    
}

bool DependencyParser::AllowLeftArc() const {
  Node *stack_top = NodeFromStack(0);
  if (stack_top->node_id() == 0)
    return false;

  if (stack_top->head_id() != Node::kNone)
    return false;

  return true;
}

bool DependencyParser::AllowReduce() const {
  if (NodeFromStack(0)->head_id() == Node::kNone)
    return false;
  else
    return true;
}

bool DependencyParser::AllowShift() const {
  return buffer_ptr_ < buffer_.size() - 1;
}

bool DependencyParser::AllowRightArc() const {
  // TODO: ...?
  return true;
  // printf("%d %d %d\n", buffer_ptr_, buffer_.size(), n_arcs_);
  if (buffer_ptr_ == buffer_.size() - 1 && n_arcs_ < buffer_.size() - 2)
    return false;
  else
    return true;
}

void DependencyParser::AddArc(Node *head, 
                              Node *dependent,
                              const char *label) {
  head->AddDependent(dependent->node_id());
  dependent->set_head_id(head->node_id());
  dependent->set_dependency_label(label);
  n_arcs_++;
}

bool DependencyParser::AllowAction(int label_id) const {
  const char *actoin_str = maxent_classifier_->yname(label_id);
  if (strncmp(actoin_str, "LARC", 4) == 0)
    return AllowLeftArc();
  else if (strncmp(actoin_str, "RARC", 4) == 0)
    return AllowRightArc();
  else if (strncmp(actoin_str, "REDU", 4) == 0)
    return AllowReduce();
  else if (strncmp(actoin_str, "SHIF", 4) == 0)
    return AllowShift();

  return true;
}

int DependencyParser::NextAction() {
  std::vector<std::pair<double, int> >
  candidate_actions(maxent_classifier_->ysize());

  BuildFeatureList();

  int yid = maxent_classifier_->Classify(
      const_cast<const char **>(feature_buffer_),
      kFeatures);
  
  // If the first candidate action is not allowed
  if (AllowAction(yid) == false) {
    // sort the results by its weight
    candidate_actions.clear();
    for (int i = 0; i < maxent_classifier_->ysize(); ++i) {
      candidate_actions.push_back(
          std::make_pair(maxent_classifier_->ycost(i), i));
    }
    std::sort(candidate_actions.begin(), candidate_actions.end());
    do {
      yid = candidate_actions.back().second;
      candidate_actions.pop_back();
    } while (AllowAction(yid) == false);
  }

  return yid;
}

void DependencyParser::MakeDepengencyInstance(DependencyInstance *dependency_instance) {
  DependencyParser::Node *node;
  for (int i = 0; i < buffer_.size() - 1; ++i) {
    node = buffer_[i + 1];    // ignore the ROOT node
    dependency_instance->set_value_at(i, 
                                      node->dependency_label(),
                                      node->head_id());
  }

  dependency_instance->set_size(buffer_.size() - 1);
}

void DependencyParser::Parse(
    DependencyInstance *dependency_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance,
    const TermInstance *term_instance) {
  Node *node;
  
  buffer_.clear();
  stack_.clear();
  n_arcs_ = 0;

  // Load the Term-POS_tag data into Dependency::Node
  node = new Node(0, "-ROOT-", "ROOT");  // ROOT node
  buffer_.push_back(node);
  
  for (int i = 0; i < term_instance->size(); ++i) {
    node = new Node(
        i + 1,    // node_id
        term_instance->term_text_at(i),
        part_of_speech_tag_instance->part_of_speech_tag_at(i));
    buffer_.push_back(node);
  }

  stack_.push_back(buffer_[0]);
  buffer_ptr_ = 1;

  while (buffer_ptr_ < buffer_.size()) {
    int label_id = NextAction();
    const char *label_str = maxent_classifier_->yname(label_id);
    if (strncmp("SHIF", label_str, 4) == 0) {
      stack_.push_back(buffer_[buffer_ptr_]);
      buffer_ptr_++;
    } else if (strncmp("REDU", label_str, 4) == 0) {
      stack_.pop_back();
    } else if (strncmp("LARC", label_str, 4) == 0) {
      AddArc(buffer_[buffer_ptr_], stack_.back(), label_str + 5);
      stack_.pop_back();
    } else if (strncmp("RARC", label_str, 4) == 0) {
      AddArc(stack_.back(), buffer_[buffer_ptr_], label_str + 5);
      stack_.push_back(buffer_[buffer_ptr_]);
      buffer_ptr_++;      
    } else {
      assert(false);
    }
  }
  
  MakeDepengencyInstance(dependency_instance);
}

}  // namespace milkcat
