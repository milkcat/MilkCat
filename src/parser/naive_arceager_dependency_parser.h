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
// naive_arceager_dependency_parser.h --- Created at 2014-09-16
//

#ifndef SRC_PARSER_NAIVE_ARCEAGER_DEPENDENCY_PARSER_H_
#define SRC_PARSER_NAIVE_ARCEAGER_DEPENDENCY_PARSER_H_

#include "parser/dependency_parser.h"
#include "common/model_impl.h"

namespace milkcat {

class Status;
class MulticlassPerceptron;
class TermInstance;
class PartOfSpeechTagInstance;
class FeatureSet;

namespace utils {
template<class T> class Pool;
}  // namespace utils

// This class implemented the original arceager dependency parser. Introduced 
// by Nivre, Joakim. 
// "Algorithms for deterministic incremental dependency parsing."
// Computational Linguistics 34.4 (2008): 513-553.
class NaiveArceagerDependencyParser: public DependencyParser {
 public:
  NaiveArceagerDependencyParser();
  ~NaiveArceagerDependencyParser();

  static NaiveArceagerDependencyParser *New(Model::Impl *model,
                                            Status *status);
  // Overrides DependencyParser::Parse
  void Parse(
    DependencyInstance *dependency_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance);

 private:
  MulticlassPerceptron *perceptron_;
  State *state_;
  Feature *feature_;
  FeatureSet *feature_set_;
  utils::Pool<Node> *node_pool_;
  const TermInstance *term_instance_;
  const PartOfSpeechTagInstance *part_of_speech_tag_instance_;

  // Choose next transition from classifier and current state
  int NextTransition();
  bool AllowTransition(int transition_id) const;

  // Stores the parsing result into dependency_instance
  void StoreResult(DependencyInstance *dependency_instance,
                   const TermInstance *term_instance,
                   const PartOfSpeechTagInstance *part_of_speech_tag_instance);
};

}  // namespace milkcat

#endif  // SRC_PARSER_NAIVE_ARCEAGER_DEPENDENCY_PARSER_H_