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
// dependency_parser_features.h --- Created at 2014-06-03
//

#include "parser/dependency_parser.h"

#include <map>
#include "utils/log.h"
#include "utils/string_builder.h"
#include "utils/utils.h"

namespace milkcat {

using utils::strlcpy;

DependencyParser::Node *
DependencyParser::NodeFromStack(int top_index) const {
  return top_index >= stack_.size()? 
      NULL: 
      stack_[stack_.size() - 1 - top_index];
}

DependencyParser::Node *
DependencyParser::NodeFromBuffer(int index) const {
  int real_index = buffer_ptr_ + index;
  return real_index >= buffer_.size()? NULL: buffer_[real_index];
}

inline DependencyParser::Node *
DependencyParser::ParentNode(Node *node) const {
  int head_id = node->head_id();
  return head_id == Node::kNone? NULL: buffer_[head_id];
}

inline DependencyParser::Node *
DependencyParser::LeftmostChildNode(Node *node) const {
  int ld = node->LeftmostChildId();
  return ld == Node::kNone? NULL: buffer_[ld];
}

inline DependencyParser::Node *
DependencyParser::RightmostChildNode(Node *node) const {
  int rd = node->RightmostChildId();
  return rd == Node::kNone? NULL: buffer_[rd];    
}

inline const char *DependencyParser::STw() {
  Node *node = NodeFromStack(0);
  if (node) 
    return node->term_str();
  else
    return "NULL";
}

inline const char *DependencyParser::STt() {
  Node *node = NodeFromStack(0);
  if (node) 
    return node->POS_tag();
  else
    return "NULL";
}

inline const char *DependencyParser::N0w() {
  Node *node = NodeFromBuffer(0);
  if (node) 
    return node->term_str();
  else
    return "NULL"; 
}

inline const char *DependencyParser::N0t() {
  Node *node = NodeFromBuffer(0);
  if (node) 
    return node->POS_tag();
  else
    return "NULL"; 
}

inline const char *DependencyParser::N1w() {
  Node *node = NodeFromBuffer(1);
  if (node) 
    return node->term_str();
  else
    return "NULL"; 
}

inline const char *DependencyParser::N1t() {
  Node *node = NodeFromBuffer(1);
  if (node) 
    return node->POS_tag();
  else
    return "NULL"; 
}

inline const char *DependencyParser::N2t() {
  Node *node = NodeFromBuffer(2);
  if (node) 
    return node->POS_tag();
  else
    return "NULL"; 
}

inline const char *DependencyParser::STPt() {
  Node *node = NodeFromStack(0);
  if (node != NULL) node = ParentNode(node);
  if (node) 
    return node->POS_tag();
  else
    return "NULL";  
}

const char *DependencyParser::STLCt() {
  Node *node = NodeFromStack(0);
  if (node != NULL) node = LeftmostChildNode(node);
  if (node) 
    return node->POS_tag();
  else
    return "NULL";
}

const char *DependencyParser::STRCt() {
  Node *node = NodeFromStack(0);
  if (node != NULL) node = RightmostChildNode(node);
  if (node) 
    return node->POS_tag();
  else
    return "NULL";
}

const char *DependencyParser::N0LCt() {
  Node *node = NodeFromBuffer(0);
  if (node != NULL) node = LeftmostChildNode(node);
  if (node) 
    return node->POS_tag();
  else
    return "NULL";
}

void DependencyParser::InitializeFeatureIndex() {
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

int DependencyParser::BuildFeature() {
  Node *node;
  utils::StringBuilder builder;
  int feature_num = 0;

  // First initialize each feature string into feature_
  strlcpy(feature_[kSTw], STw(), kFeatureStringMax);
  strlcpy(feature_[kSTt], STt(), kFeatureStringMax);
  strlcpy(feature_[kN0w], N0w(), kFeatureStringMax);
  strlcpy(feature_[kN0t], N0t(), kFeatureStringMax);
  strlcpy(feature_[kN1w], N1w(), kFeatureStringMax);
  strlcpy(feature_[kN1t], N1t(), kFeatureStringMax);
  strlcpy(feature_[kN2t], N2t(), kFeatureStringMax);
  strlcpy(feature_[kSTPt], STPt(), kFeatureStringMax);
  strlcpy(feature_[kSTLCt], STLCt(), kFeatureStringMax);
  strlcpy(feature_[kSTRCt], STRCt(), kFeatureStringMax);
  strlcpy(feature_[kN0LCt], N0LCt(), kFeatureStringMax);

  for (std::vector<std::string>::iterator
       it = feature_templates_.begin(); it != feature_templates_.end(); ++it) {
    builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
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
        builder << feature_[fid];
        p = q + 1;
      }
    }
    LOG(feature_buffer_[feature_num]);
    feature_num++;
  }

  return feature_num;
}

}  // namespace milkcat
