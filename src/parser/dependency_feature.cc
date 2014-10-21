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
// dependency_parser_features.cc --- Created at 2014-06-03
// dependency_feature.cc --- Created at 2014-09-15
//

#include "parser/dependency_feature.h"

#include <map>
#include "common/trie_tree.h"
#include "ml/feature_set.h"
#include "parser/dependency_node.h"
#include "parser/dependency_state.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/log.h"
#include "utils/string_builder.h"

namespace milkcat {

const char *DependencyParser::Feature::kRootTerm = "ROOT";
const char *DependencyParser::Feature::kRootTag = "ROOT";

DependencyParser::Feature::Feature(
    const std::vector<std::string> *feature_templates): 
        term_instance_(NULL),
        part_of_speech_tag_instance_(NULL),
        state_(NULL),
        feature_index_(NULL),
        feature_templates_(feature_templates) {
  InitializeFeatureIndex();
}

DependencyParser::Feature::~Feature() {
  delete feature_index_;
  feature_index_ = NULL;
}

inline const char *DependencyParser::Feature::Tag(const Node *node) {
  if (node == NULL) return "NULL";
  if (node->id() == 0) return kRootTag;
  return part_of_speech_tag_instance_->part_of_speech_tag_at(node->id() - 1);
}

inline const char *DependencyParser::Feature::Term(const Node *node) {
  if (node == NULL) return "NULL";
  if (node->id() == 0) return kRootTerm;
  return term_instance_->term_text_at(node->id() - 1);  
}

inline const char *DependencyParser::Feature::STw() {
  const Node *node = state_->Stack(0);
  return Term(node);
}

inline const char *DependencyParser::Feature::STt() {
  const Node *node = state_->Stack(0);
  return Tag(node);
}

inline const char *DependencyParser::Feature::N0w() {
  const Node *node = state_->Input(0);
  return Term(node);
}

inline const char *DependencyParser::Feature::N0t() {
  const Node *node = state_->Input(0);
  return Tag(node);
}

inline const char *DependencyParser::Feature::N1w() {
  const Node *node = state_->Input(1);
  return Term(node);
}

inline const char *DependencyParser::Feature::N1t() {
  const Node *node = state_->Input(1);
  return Tag(node);
}

inline const char *DependencyParser::Feature::N2t() {
  const Node *node = state_->Input(2);
  return Tag(node);
}

inline const char *DependencyParser::Feature::STPt() {
  const Node *node = state_->Stack(0);
  node = state_->Parent(node);
  return Tag(node);
}

const char *DependencyParser::Feature::STLCt() {
  const Node *node = state_->Stack(0);
  node = state_->LeftChild(node);
  return Tag(node);
}

const char *DependencyParser::Feature::STRCt() {
  const Node *node = state_->Stack(0);
  node = state_->RightChild(node);
  return Tag(node);
}

const char *DependencyParser::Feature::N0LCt() {
  const Node *node = state_->Input(0);
  node = state_->LeftChild(node);
  return Tag(node);
}

void DependencyParser::Feature::InitializeFeatureIndex() {
  std::map<std::string, int> feature_index;
  feature_index["STw"] = kSTw;
  feature_index["STt"] = kSTt;
  feature_index["N0w"] = kN0w;
  feature_index["N0t"] = kN0t;
  feature_index["N1w"] = kN1w;
  feature_index["N1t"] = kN1t;
  feature_index["N2t"] = kN2t;
  feature_index["STPt"] = kSTPt;
  feature_index["STLCt"] = kSTLCt;
  feature_index["STRCt"] = kSTRCt;
  feature_index["N0LCt"] = kN0LCt;

  feature_index_ = DoubleArrayTrieTree::NewFromMap(feature_index);
}

int DependencyParser::Feature::Extract(
    const State *state,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance,
    FeatureSet *feature_set) {
  Node *node;
  utils::StringBuilder builder;
  int feature_num = 0;

  state_ = state;
  term_instance_ = term_instance;
  part_of_speech_tag_instance_ = part_of_speech_tag_instance;

  // First initialize each feature string into single_feature_
  strlcpy(single_feature_[kSTw], STw(), kFeatureStringMax);
  strlcpy(single_feature_[kSTt], STt(), kFeatureStringMax);
  strlcpy(single_feature_[kN0w], N0w(), kFeatureStringMax);
  strlcpy(single_feature_[kN0t], N0t(), kFeatureStringMax);
  strlcpy(single_feature_[kN1w], N1w(), kFeatureStringMax);
  strlcpy(single_feature_[kN1t], N1t(), kFeatureStringMax);
  strlcpy(single_feature_[kN2t], N2t(), kFeatureStringMax);
  strlcpy(single_feature_[kSTPt], STPt(), kFeatureStringMax);
  strlcpy(single_feature_[kSTLCt], STLCt(), kFeatureStringMax);
  strlcpy(single_feature_[kSTRCt], STRCt(), kFeatureStringMax);
  strlcpy(single_feature_[kN0LCt], N0LCt(), kFeatureStringMax);

  feature_set->Clear();
  for (std::vector<std::string>::const_iterator
       it = feature_templates_->begin(); it != feature_templates_->end(); ++it) {
    builder.ChangeBuffer(feature_set->at(feature_num),
                         FeatureSet::kFeatureSizeMax);
    const char *templ = it->c_str();
    const char *p = templ, *q = NULL;
    while (*p) {
      if (*p != '[') {
        builder << *p;
        p++;
      } else {
        const char *q = p;
        while (*q != ']') {
          if (*q == '\0') ERROR("Template file corrputed.");
          ++q;
        }
        int len = q - p - 1;
        int fid = feature_index_->Search(p + 1, len);
        if (fid < 0) ERROR("Template file corrputed.");
        builder << single_feature_[fid];
        p = q + 1;
      }
    }
    LOG(feature_[feature_num]);
    feature_num++;
  }
  feature_set->set_size(feature_num);

  return feature_num;
}

}  // namespace milkcat