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

#include "utils/log.h"
#include "utils/string_builder.h"

namespace milkcat {


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

int DependencyParser::BuildFeature() {
  Node *node;
  utils::StringBuilder builder;
  int feature_num = 0;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STwt:" << STw() << '/' << STt();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STw:" << STw();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STt:" << STt();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N0wt:" << N0w() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N0w:" << N0w();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N0t:" << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N1wt:" << N1w() << '/' << N1t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N1w:" << N1w();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N1t:" << N1t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STwtN0wt:" << STw() << '/' << STt() << '/' << N0w() << '/'
          << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STwtN0w:" << STw() << '/' << STt() << '/' << N0w();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STwN0wt:" << STw() << '/' << N0w() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STwtN0t:" << STw() << '/' << STt() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtN0wt:" << STt() << '/' << N0w() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STwN0w:" << STw() << '/' << N0w();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtN0t:" << STt() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N0tN1t:" << N0t() << '/' << N1t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N0tN1tN2t:" << N0t() << '/' << N1t() << '/' << N2t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtN0tN1t:" << STt() << '/' << N0t() << '/' << N1t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STPtSTtN0t:" << STPt() << '/' << STt() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtSTLCtN0t:" << STt() << '/' << STLCt() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtSTRCtN0t:" << STt() << '/' << STRCt() << '/' << N0t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtN0tN0LCt:" << STt() << '/' << N0t() << '/' << N0LCt();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "N0wN1tN2t:" << N0w() << '/' << N1t() << '/' << N2t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtN0wN1t:" << STt() << '/' << N0w() << '/' << N1t();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STPtSTtN0w:" << STPt() << '/' << STt() << '/' << N0w();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtSTLCtN0w:" << STt() << '/' << STLCt() << '/' << N0w();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtSTRCtN0w:" << STt() << '/' << STRCt() << '/' << N0w();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  builder.ChangeBuffer(feature_buffer_[feature_num], kFeatureStringMax);
  builder << "STtN0wN0LCt:" << STt() << '/' << N0w() << '/' << N0LCt();
  LOG(feature_buffer_[feature_num]);
  feature_num++;

  return feature_num;
}

}  // namespace milkcat