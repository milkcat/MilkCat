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
// yamada_parser.h --- Created at 2015-01-27
//

#ifndef SRC_PARSER_YAMADA_PARSER_H_
#define SRC_PARSER_YAMADA_PARSER_H_

#include <vector>
#include <string>
#include "parser/dependency_parser.h"

namespace milkcat {

class Status;
class PerceptronModel;
class Perceptron;
class TermInstance;
class PartOfSpeechTagInstance;
class FeatureSet;
template<class T> class Pool;
class Model;

// This class implemented the original Yamada (arc-standard) dependency parser.
// Introduced by Nivre, Joakim. 
// "Algorithms for deterministic incremental dependency parsing."
// Computational Linguistics 34.4 (2008): 513-553.
class YamadaParser: public DependencyParser {
 public:
  YamadaParser(
      PerceptronModel *perceptron_model,
      FeatureTemplate *feature);
  ~YamadaParser();

  static YamadaParser *New(Model *model, Status *status);
  // Overrides DependencyParser::Parse
  void Parse(
      TreeInstance *tree_instance,
      const TermInstance *term_instance,
      const PartOfSpeechTagInstance *part_of_speech_tag_instance);

 private:
  State *state_;

  // Predict next transition from current state
  int Next();

  // Do the transition `yid` and step to next state
  void Step(int yid);

  // Do some preparing work
  void Start(const TermInstance *term_instance,
             const PartOfSpeechTagInstance *part_of_speech_tag_instance);

  // Stores the parsing result into tree_instance
  void StoreResult(TreeInstance *tree_instance,
                   const TermInstance *term_instance,
                   const PartOfSpeechTagInstance *part_of_speech_tag_instance);
};

}  // namespace milkcat

#endif  // SRC_PARSER_YAMADA_PARSER_H_
