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
// beam_arceager_dependency_parser.h --- Created at 2014-10-31
// beam_yamada_parser.h --- Created at 2015-01-27
//

#ifndef SRC_PARSER_BEAM_YAMADA_PARSER_H_
#define SRC_PARSER_BEAM_YAMADA_PARSER_H_

#include <stdint.h>
#include <vector>
#include <string>
#include "common/model_impl.h"
#include "ml/beam.h"
#include "parser/dependency_parser.h"

namespace milkcat {

class Status;
class PerceptronModel;
class Perceptron;
class TermInstance;
class PartOfSpeechTagInstance;
class ReimuTrie;
class FeatureSet;
template<class T> class Pool;
template<class T> class StaticArray;

class BeamYamadaParser: public DependencyParser {
 public:
  BeamYamadaParser(
      PerceptronModel *perceptron_model,
      FeatureTemplate *feature,
      int beam_size);
  ~BeamYamadaParser();

  // Training the BeamYamadaParser from `training_corpus` with
  // the feature template from `template_filename` and stores the model into
  // `model_prefix`
  static void Train(
      const char *training_corpus,
      const char *template_filename,
      const char *model_prefix,
      int max_iteration,
      int beam_size,
      Status *status);

  static BeamYamadaParser *New(Model::Impl *model, Status *status);

  // Overrides DependencyParser::Parse
  void Parse(
      TreeInstance *tree_instance,
      const TermInstance *term_instance,
      const PartOfSpeechTagInstance *part_of_speech_tag_instance);

 private:
  class StateCmp;

  Pool<State> *state_pool_;
  float *agent_;
  Beam<State, StateCmp> *beam_;
  Beam<State, StateCmp> *next_beam_;
  int agent_size_;
  int beam_size_;

  // Copys `state` and applies transition `yid` to the new state
  State *StateCopyAndMove(State *state, int yid);

  // Step to next transtions. Returns false indicates the end reached
  bool Step();

  // Start to parse the sentence
  void Start(const TermInstance *term_instance,
             const PartOfSpeechTagInstance *part_of_speech_tag_instance);

  // Stores the parsing result into tree_instance
  void StoreResult(TreeInstance *tree_instance);

  // Extracts features from `state` and put into `feature_set`
  void ExtractFeatureFromState(const State* state, FeatureSet *feature_set);

  // Training `perceptron` with an correct (orcale) and incorrect state pair
  static void UpdateWeightForState(
      State *incorrect_state,
      State *orcale_state,
      BeamYamadaParser *parser,
      Perceptron *percpetron);
};

}  // namespace milkcat

#endif  // SRC_PARSER_BEAM_YAMADA_PARSER_H_
