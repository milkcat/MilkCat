/*
 * dependency_parsing.h
 *
 * by ling0322 at 2013-08-10
 *
 */



#ifndef DEPENDENCY_PARSING_H
#define DEPENDENCY_PARSING_H

#include <set>
#include <vector>
#include <string.h>
#include <assert.h>
#include "common/model_factory.h"

namespace milkcat {

class TermInstance;
class PartOfSpeechTagInstance;
class DependencyInstance;
class MaxentClassifier;
class Status;

class DependencyParser {
 public:
  class Node;

  static const int kFeatures = 19;
  static const int kFeatureStringMax = 1000;

  static DependencyParser *New(Model::Impl *model_impl, Status *status);
  ~DependencyParser();

  void Parse(
    DependencyInstance *dependency_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance,
    const TermInstance *term_instance);

 private:
  DependencyParser();

  // Gets the buffer_ptr_ + index node from buffer_, returns the pointer of 
  // the node or NULL if the index is out-of-boundary 
  Node *NodeFromBuffer(int index) const;

  // Gets the nth-top node from stack, returns the pointer of the node or NULL
  // if the top_index is out-of-boundary
  Node *NodeFromStack(int top_index) const;

  // Get the related node for the current node
  Node *HeadNode(Node *node) const;
  Node *LeftmostDependentNode(Node *node) const;
  Node *RightmostDependentNode(Node *node) const;

  // Check if current state allows an action
  bool AllowLeftArc() const;
  bool AllowReduce() const;
  bool AllowShift() const;
  bool AllowRightArc() const;

  // Builds the features from current state and stores it in feature_buffer_
  void BuildFeatureList();

  // Check if the action defined by label_id is allowed
  bool AllowAction(int label_id) const;

  // Return the label id for next action determined by CRF classifier and rules
  int NextAction();

  // Adds an arc from head to dependent of the type specified by type
  void AddArc(Node *head, Node *dependent, const char *type);

  // Stores the result into dependency_instance
  void MakeDepengencyInstance(DependencyInstance *dependency_instance);

  std::vector<Node *> buffer_;
  std::vector<Node *> stack_;
  int buffer_ptr_;
  int n_arcs_;
  MaxentClassifier *maxent_classifier_;
  char **feature_buffer_;
};


class DependencyParser::Node {
 public:
  static const int kNone = -1;

  Node(int node_id, const char *term_str, const char *POS_tag):
      node_id_(node_id),
      head_id_(kNone),
      dependency_ids_(std::set<int>()) {
    strncpy(term_str_, term_str, kTermLengthMax);
    strcpy(dependency_label_, "NULL");
    strncpy(POS_tag_, POS_tag, kPOSTagLengthMax);
  }


  int node_id() const { return node_id_; }
  const char *term_str() const { return term_str_; }
  const char *POS_tag() const { return POS_tag_; }
  int head_id() const { return head_id_; }

  void set_head_id(int head_id) { 
    assert(head_id_ == kNone);
    head_id_ = head_id; 
  }

  const char *dependency_label() const { return dependency_label_; }

  void set_dependency_label(const char *label) {
    strncpy(dependency_label_, label, sizeof(dependency_label_) - 1);
  }
  
  // Get the leftmost depentent's id of current node
  int GetLeftmostDepententId() const {

    // dependency_ids_ is sorted
    if (dependency_ids_.size() == 0)
      return kNone;
    else
      return *dependency_ids_.begin();
  }

  int GetRightmostDependentId() const {
    if (dependency_ids_.size() == 0)
      return kNone;
    else
      return *dependency_ids_.rbegin();
  }
  
  // Add a dependent to this node
  void AddDependent(int depentent_id) {
    dependency_ids_.insert(depentent_id);
  }


 private:
  int node_id_;
  char term_str_[kTermLengthMax];
  char POS_tag_[kPOSTagLengthMax];
  int head_id_;
  std::set<int> dependency_ids_;
  char dependency_label_[10];
};

}  // namespace milkcat

#endif