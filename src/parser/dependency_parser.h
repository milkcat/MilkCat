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
#include "common/trie_tree.h"

namespace utils {

class StringBuilder;

}  // namespace utils

namespace milkcat {

class TermInstance;
class PartOfSpeechTagInstance;
class DependencyInstance;
class MaxentClassifier;
class Status;

// Use the arc-eager algorithm to parse a sentence into a dependency tree.
class DependencyParser {
 public:
  class Node;

  static const int kFeatureMax = 50;
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
  Node *ParentNode(Node *node) const;
  Node *LeftmostChildNode(Node *node) const;
  Node *RightmostChildNode(Node *node) const;

  // The features used in dependency parsing
  enum {
    kSTw = 0,
    kSTt,
    kN0w,
    kN0t,
    kN1w,
    kN1t,
    kN2t,
    kSTPt,
    kSTLCt,
    kSTRCt,
    kN0LCt,
    kFeatureNumber
  };

  // Extract the feature from current configuration. The feature are defined in
  // Zhang Yue, A Tale of Two Parsers: investigating and combining graph-based 
  // and transition-based dependency parsing using beam-search, 2008
  const char *STw();
  const char *STt();
  const char *N0w();
  const char *N0t();
  const char *N1w();
  const char *N1t();
  const char *N2t();
  const char *STPt();
  const char *STLCt();
  const char *STRCt();
  const char *N0LCt();

  // Check if current state allows an action
  bool AllowLeftArc() const;
  bool AllowReduce() const;
  bool AllowShift() const;
  bool AllowRightArc() const;

  // Builds the features from current state and stores it in feature_buffer_
  // Returns the number of features added
  int BuildFeature();

  // Get the cost value from one feature template and current state
  float CostFromTemplate(const char *feature_template);

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

  // Initialize the feature index, store the key-value pair in feature_index_
  void InitializeFeatureIndex();

  // Index for single feature, such as STw, STt ...
  TrieTree *feature_index_;
  std::vector<std::string> feature_templates_;

  int right_verb_count_[kTermMax + 1];
  int buffer_ptr_;
  bool have_root_node_;
  int n_arcs_;
  MaxentClassifier *maxent_classifier_;
  char **feature_buffer_;
  char feature_[kFeatureNumber][kFeatureStringMax];
  int last_transition_;
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
  
  // Get the leftmost child's id of current node
  int LeftmostChildId() const {
    if (dependency_ids_.size() > 0) {
      int left_child = *dependency_ids_.begin();
      if (left_child < node_id_) return left_child;
    }
    return kNone;
  }

  int RightmostChildId() const {
    if (dependency_ids_.size() > 0) {
      int right_child = *dependency_ids_.begin();
      if (right_child > node_id_) return right_child;
    }
    return kNone;
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