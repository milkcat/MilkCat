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
// dependency_feature.h --- Created at 2014-09-15
//

#ifndef SRC_PARSER_DEPENDENCY_FEATURE_H_
#define SRC_PARSER_DEPENDENCY_FEATURE_H_

#include <vector>
#include "parser/dependency_parser.h"

namespace milkcat {

class TermInstance;
class PartOfSpeechTagInstance;
class TrieTree;

class DependencyParser::Feature {
 public:
  static const int kFeatureMax = 50;
  static const int kFeatureStringMax = 1000;

  static const char *kRootTerm;
  static const char *kRootTag;

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
    kSingleFeatureNumber
  };

  Feature(const std::vector<std::string> *feature_templates);
  ~Feature();

  // Returns the tag or term string of a node
  const char *Tag(const Node *node);
  const char *Term(const Node *node);

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

  // Get the feature at index fid
  const char *feature(int fid) const { return feature_[fid]; }

  // Builds the features from current state and stores it in feature_buffer_
  // Returns the number of features added
  int BuildFeature(const State *state,
                   const TermInstance *term_instance,
                   const PartOfSpeechTagInstance *part_of_speech_tag_instance);

 private:
  char feature_[kFeatureMax][kFeatureStringMax];
  char single_feature_[kSingleFeatureNumber][kFeatureStringMax];
  const TermInstance *term_instance_;
  const PartOfSpeechTagInstance *part_of_speech_tag_instance_;
  const State *state_;
  TrieTree *feature_index_;
  const std::vector<std::string> *feature_templates_;

  void InitializeFeatureIndex();
};

}  // namespace milkcat

#endif  // SRC_PARSER_DEPENDENCY_FEATURE_H_