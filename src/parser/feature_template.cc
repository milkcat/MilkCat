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
// feature_template.cc --- Created at 2014-10-27
//

#include "parser/feature_template.h"

#include <stdio.h>
#include <map>
#include "common/trie_tree.h"
#include "ml/feature_set.h"
#include "parser/node.h"
#include "parser/state.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/readable_file.h"
#include "utils/string_builder.h"

namespace milkcat {

const char *DependencyParser::FeatureTemplate::kRootTerm = "ROOT";
const char *DependencyParser::FeatureTemplate::kRootTag = "ROOT";

DependencyParser::FeatureTemplate::FeatureTemplate(
    const std::vector<std::string> &feature_template): 
        term_instance_(NULL),
        part_of_speech_tag_instance_(NULL),
        state_(NULL),
        feature_index_(NULL),
        feature_template_(feature_template) {
  InitializeFeatureIndex();
}

DependencyParser::FeatureTemplate *
DependencyParser::FeatureTemplate::Open(const char *filename, Status *status) {
  // Read template file
  char line[1024];
  std::vector<std::string> template_vector;
  ReadableFile *fd = ReadableFile::New(filename, status);

  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      trim(line);
      template_vector.push_back(line);
    }
  }
  delete fd;
  fd = NULL;

  if (status->ok()) {
    DependencyParser::FeatureTemplate *
    self = new DependencyParser::FeatureTemplate(template_vector);
    return self;
  } else {
    return NULL;
  }
}

DependencyParser::FeatureTemplate::~FeatureTemplate() {
  delete feature_index_;
  feature_index_ = NULL;
}

inline const char *DependencyParser::FeatureTemplate::Tag(const Node *node) {
  if (node == NULL) return "NULL";
  if (node->id() == 0) return kRootTag;
  return part_of_speech_tag_instance_->part_of_speech_tag_at(node->id() - 1);
}

inline const char *DependencyParser::FeatureTemplate::Term(const Node *node) {
  if (node == NULL) return "NULL";
  if (node->id() == 0) return kRootTerm;
  return term_instance_->term_text_at(node->id() - 1);  
}

inline const char *DependencyParser::FeatureTemplate::STw() {
  const Node *node = state_->Stack(0);
  return Term(node);
}

inline const char *DependencyParser::FeatureTemplate::STt() {
  const Node *node = state_->Stack(0);
  return Tag(node);
}

inline const char *DependencyParser::FeatureTemplate::N0w() {
  const Node *node = state_->Input(0);
  return Term(node);
}

inline const char *DependencyParser::FeatureTemplate::N0t() {
  const Node *node = state_->Input(0);
  return Tag(node);
}

inline const char *DependencyParser::FeatureTemplate::N1w() {
  const Node *node = state_->Input(1);
  return Term(node);
}

inline const char *DependencyParser::FeatureTemplate::N1t() {
  const Node *node = state_->Input(1);
  return Tag(node);
}

inline const char *DependencyParser::FeatureTemplate::N2t() {
  const Node *node = state_->Input(2);
  return Tag(node);
}

inline const char *DependencyParser::FeatureTemplate::STPt() {
  const Node *node = state_->Stack(0);
  node = state_->Parent(node);
  return Tag(node);
}

const char *DependencyParser::FeatureTemplate::STLCt() {
  const Node *node = state_->Stack(0);
  node = state_->LeftChild(node);
  return Tag(node);
}

const char *DependencyParser::FeatureTemplate::STRCt() {
  const Node *node = state_->Stack(0);
  node = state_->RightChild(node);
  return Tag(node);
}

const char *DependencyParser::FeatureTemplate::N0LCt() {
  const Node *node = state_->Input(0);
  node = state_->LeftChild(node);
  return Tag(node);
}

void DependencyParser::FeatureTemplate::InitializeFeatureIndex() {
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

int DependencyParser::FeatureTemplate::Extract(
    const State *state,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance,
    FeatureSet *feature_set) {
  Node *node;
  StringBuilder builder;
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
       it = feature_template_.begin();
       it != feature_template_.end();
       ++it) {
    builder.SetOutput(feature_set->at(feature_num), FeatureSet::kFeatureSizeMax);
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
    feature_num++;
  }
  feature_set->set_size(feature_num);

  return feature_num;
}

}  // namespace milkcat