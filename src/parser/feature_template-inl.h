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
// feature_template-inl.h --- Created at 2014-12-26
//

#ifndef SRC_PARSER_FEATURE_TEMPLATE_INL_H_
#define SRC_PARSER_FEATURE_TEMPLATE_INL_H_

#include "parser/feature_template.h"

#include "parser/node.h"
#include "parser/state.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"

namespace milkcat {

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

inline const char *DependencyParser::FeatureTemplate::STLCt() {
  const Node *node = state_->Stack(0);
  node = state_->LeftChild(node);
  return Tag(node);
}

inline const char *DependencyParser::FeatureTemplate::STRCt() {
  const Node *node = state_->Stack(0);
  node = state_->RightChild(node);
  return Tag(node);
}

inline const char *DependencyParser::FeatureTemplate::N0LCt() {
  const Node *node = state_->Input(0);
  node = state_->LeftChild(node);
  return Tag(node);
}

inline const char *DependencyParser::FeatureTemplate::N0RCt() {
  const Node *node = state_->Input(0);
  node = state_->RightChild(node);
  return Tag(node);
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

}  // namespace milkcat 

#endif  // SRC_PARSER_FEATURE_TEMPLATE_INL_H_
