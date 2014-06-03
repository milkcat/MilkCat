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

#include "milkcat/dependency_parser.h"

#include "utils/string_builder.h"

namespace milkcat {

void DependencyParser::BuildFeatureList() {
  Node *node;
  utils::StringBuilder builder;

  // F00: word(buffer[0])
  node = NodeFromBuffer(0);
  builder.ChangeBuffer(feature_buffer_[0], kFeatureStringMax);
  builder << "B0:";
  if (node) 
    builder << node->term_str();
  else
    builder << "NULL";

  // F01: word(buffer[1])
  node = NodeFromBuffer(1);
  builder.ChangeBuffer(feature_buffer_[1], kFeatureStringMax);
  builder << "B1:";
  if (node) 
    builder << node->term_str();
  else
    builder << "NULL";

  // F02: word(stack[0])
  node = NodeFromStack(0);
  builder.ChangeBuffer(feature_buffer_[2], kFeatureStringMax);
  builder << "S0:";
  if (node) 
    builder << node->term_str();
  else
    builder << "NULL";

  // F03: pos(buffer[0])
  node = NodeFromBuffer(0);
  builder.ChangeBuffer(feature_buffer_[3], kFeatureStringMax);
  builder << "B0T:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F04: pos(buffer[1])
  node = NodeFromBuffer(1);
  builder.ChangeBuffer(feature_buffer_[4], kFeatureStringMax);
  builder << "B1T:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F05: pos(buffer[2])
  node = NodeFromBuffer(2);
  builder.ChangeBuffer(feature_buffer_[5], kFeatureStringMax);
  builder << "B2T:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F06: pos(buffer[3])
  node = NodeFromBuffer(3);
  builder.ChangeBuffer(feature_buffer_[6], kFeatureStringMax);
  builder << "B3T:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F07: pos(stack[0])
  node = NodeFromStack(0);
  builder.ChangeBuffer(feature_buffer_[7], kFeatureStringMax);
  builder << "B3T:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F08: pos(stack[1])
  node = NodeFromStack(1);
  builder.ChangeBuffer(feature_buffer_[8], kFeatureStringMax);
  builder << "S0T:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F09: dep(ld(buffer[0]))
  node = NodeFromBuffer(0);
  if (node != NULL) node = LeftmostDependentNode(node);
  builder.ChangeBuffer(feature_buffer_[9], kFeatureStringMax);
  builder << "B0LD:";
  if (node) 
    builder << node->dependency_label();
  else
    builder << "NULL"; 

  // F10: dep(stack[0])
  node = NodeFromStack(0);
  builder.ChangeBuffer(feature_buffer_[10], kFeatureStringMax);
  builder << "S0D:";
  if (node) 
    builder << node->dependency_label();
  else
    builder << "NULL";

  // F11: word(hd(stack[0]))
  node = NodeFromStack(0);
  if (node != NULL) node = HeadNode(node);
  builder.ChangeBuffer(feature_buffer_[11], kFeatureStringMax);
  builder << "S0H:";
  if (node) 
    builder << node->term_str();
  else
    builder << "NULL";

  // F12: dep(ld(stack[0]))
  node = NodeFromStack(0);
  if (node != NULL) node = LeftmostDependentNode(node);
  builder.ChangeBuffer(feature_buffer_[12], kFeatureStringMax);
  builder << "S0LD:";
  if (node) 
    builder << node->dependency_label();
  else
    builder << "NULL";

  // F13: dep(rd(stack[0]))
  node = NodeFromStack(0);
  if (node != NULL) node = RightmostDependentNode(node);
  builder.ChangeBuffer(feature_buffer_[13], kFeatureStringMax);
  builder << "S0RD:";
  if (node) 
    builder << node->dependency_label();
  else
    builder << "NULL";

  // F14: pos(stack[0])/word(buffer[0])
  builder.ChangeBuffer(feature_buffer_[14], kFeatureStringMax);
  node = NodeFromStack(0);
  builder << "S0TB0:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(0);
  if (node) 
    builder << node->term_str();
  else
    builder << "NULL";

  // F15: word(stack[0])/pos(buffer[0])
  builder.ChangeBuffer(feature_buffer_[15], kFeatureStringMax);
  node = NodeFromStack(0);
  builder << "S0TB0:";
  if (node) 
    builder << node->term_str();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(0);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F16: pos(stack[0])/pos(buffer[0])
  builder.ChangeBuffer(feature_buffer_[16], kFeatureStringMax);
  node = NodeFromStack(0);
  builder << "BST2:";
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(0);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F17: Part-of-speech tag 4-gram 
  builder.ChangeBuffer(feature_buffer_[17], kFeatureStringMax);
  builder << "BST4:";
  node = NodeFromStack(1);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromStack(0);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(0);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(1);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";

  // F18: Part-of-speech tag 6-gram 
  builder.ChangeBuffer(feature_buffer_[18], kFeatureStringMax);
  builder << "BST4:";
  node = NodeFromStack(1);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromStack(0);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(0);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(1);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(2);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
  builder << '/';
  node = NodeFromBuffer(3);
  if (node) 
    builder << node->POS_tag();
  else
    builder << "NULL";
}

}  // namespace milkcat