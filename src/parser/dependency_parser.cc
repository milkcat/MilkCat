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

#include "parser/dependency_parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include "ml/maxent_classifier.h"
#include "segmenter/term_instance.h"
#include "parser/dependency_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/status.h"
#include "utils/string_builder.h"
#include "utils/utils.h"

namespace milkcat {


DependencyParser::DependencyParser(): feature_index_(NULL),
                                      maxent_classifier_(NULL),
                                      feature_buffer_(NULL),
                                      last_transition_(0) {
}

DependencyParser *DependencyParser::New(Model::Impl *model_impl,
                                        Status *status) {
  DependencyParser *self = new DependencyParser();
  const MaxentModel *maxent_model = model_impl->DependencyModel(status);
  const std::vector<std::string> *feature_templates = NULL;

  if (status->ok()) {
    self->InitializeFeatureIndex();
    self->maxent_classifier_ = new MaxentClassifier(maxent_model);
    self->feature_buffer_ = new char *[kFeatureMax];
    for (int i = 0; i < kFeatureMax; ++i) {
      self->feature_buffer_[i] = new char[kFeatureStringMax];
    }
    feature_templates = model_impl->DependencyTemplate(status);
  }

  if (status->ok()) self->feature_templates_ = *feature_templates;

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

DependencyParser::~DependencyParser() {
  delete maxent_classifier_;
  maxent_classifier_ = NULL;

  if (feature_buffer_ != NULL) {
    for (int i = 0; i < kFeatureMax; ++i)
      delete[] feature_buffer_[i];

    delete[] feature_buffer_; 
    feature_buffer_ = NULL;
  }

  delete feature_index_;
  feature_index_ = NULL;
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
  // if (strcmp(maxent_classifier_->yname(last_transition_), "REDU") == 0)
  //  return false;
  return buffer_ptr_ < buffer_.size() - 1;
}

bool DependencyParser::AllowRightArc() const {
  LOG(stack_.size() << " " << have_root_node_);
  if (stack_.size() == 1 && have_root_node_ == true) return false;
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
  LOG("LabelId: " << label_id);
  const char *actoin_str = maxent_classifier_->yname(label_id);
  if (strncmp(actoin_str, "LARC", 4) == 0)
    return AllowLeftArc();
  else if (strncmp(actoin_str, "RARC", 4) == 0)
    return AllowRightArc();
  else if (strncmp(actoin_str, "REDU", 4) == 0)
    return AllowReduce();
  else if (strncmp(actoin_str, "SHIF", 4) == 0)
    return AllowShift();
  assert(false);

  return true;
}

int DependencyParser::NextAction() {
  std::vector<std::pair<double, int> >
  candidate_actions(maxent_classifier_->ysize());

  int feature_num = BuildFeature();

  int yid = maxent_classifier_->Classify(
      const_cast<const char **>(feature_buffer_),
      feature_num);
  
  LOG("Original action: " << maxent_classifier_->yname(yid));

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
      if (candidate_actions.size() == 0) {
        yid = 0; // SHIFT
        break;
      }
    } while (AllowAction(yid) == false);
  }

  LOG("Action: " << maxent_classifier_->yname(yid) << " (" << yid << ")");

  last_transition_ = yid;
  if (strcmp(maxent_classifier_->yname(yid), "RARC-ROOT") == 0)
    have_root_node_ = true;
  return yid;
}

void DependencyParser::MakeDepengencyInstance(
    DependencyInstance *dependency_instance) {
  Node *node;
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
  last_transition_ = 0;
  have_root_node_ = false;

  // Load the Term-POS_tag data into Dependency::Node
  node = new Node(0, "-ROOT-", "ROOT");  // ROOT node
  buffer_.push_back(node);
  
  for (int i = 0; i < term_instance->size(); ++i) {
    // TODO: Remove CR/LF Here
    LOG(i << " " << term_instance->size());
    node = new Node(
        i + 1,    // node_id
        term_instance->term_text_at(i),
        part_of_speech_tag_instance->part_of_speech_tag_at(i));
    buffer_.push_back(node);
  }

  // Build the right verb count feature
  int verb_count = 0;
  for (int i = term_instance->size() - 1; i >= 0; --i) {
    right_verb_count_[i + 1] = verb_count;
    if (*part_of_speech_tag_instance->part_of_speech_tag_at(i) == 'V')
      verb_count++;
  }
  right_verb_count_[0] = verb_count;

  // The root node
  stack_.push_back(buffer_[0]);
  buffer_ptr_ = 1;
  while (buffer_ptr_ < buffer_.size()) {
    int label_id = NextAction();
    LOG("Stack size: " << stack_.size());
    LOG("Buffer size: " << buffer_.size() - buffer_ptr_);
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
      LOG("Unknown transition: " << label_str);
      assert(false);
    }
  }
  
  MakeDepengencyInstance(dependency_instance);
}

}  // namespace milkcat
