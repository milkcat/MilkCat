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
// hmm_part_of_speech_tagger.h --- Created at 2013-11-08
//

#ifndef SRC_TAGGER_HMM_PART_OF_SPEECH_TAGGER_H_
#define SRC_TAGGER_HMM_PART_OF_SPEECH_TAGGER_H_

#include "common/milkcat_config.h"
#include "ml/hmm_model.h"
#include "tagger/part_of_speech_tagger.h"
#include "util/util.h"

namespace milkcat {

class PartOfSpeechTagInstance;
class TermInstance;
template<class T, class Comparator> class Beam;
class Model;
template<class T> class Pool;

// HMMPartOfSpeechTagger uses Hidden Markov Model to predict the part-of-speech
// tag of given TermInstance
class HMMPartOfSpeechTagger: public PartOfSpeechTagger {
 public:
  struct Node;

  // Beam size, two position for BOS node
  static const int kMaxBeams = kTokenMax + 2;
  static const int kBeamSize = 3;

  ~HMMPartOfSpeechTagger();
  void Tag(PartOfSpeechTagInstance *part_of_speech_tag_instance,
           TermInstance *term_instance);

  static HMMPartOfSpeechTagger *New(Model *model_factory, Status *status);
  static HMMPartOfSpeechTagger *New(const HMMModel *model, Status *status);

  // Trains the tagger from `training_corpus` and save this model into
  // `model_filename`
  static void Train(const char *training_corpus,
                    const char *model_filename,
                    Status *status);

 private:
  class NodeComparator;

  Beam<Node, NodeComparator> *beams_[kMaxBeams];
  Pool<Node> *node_pool_;

  const HMMModel *model_;

  HMMModel::EmissionArray *PU_emission_;
  HMMModel::EmissionArray *CD_emission_;
  HMMModel::EmissionArray *NN_emission_;
  HMMModel::EmissionArray *BOS_emission_;

  TermInstance *term_instance_;

  HMMPartOfSpeechTagger();

  // Step the viterbi decoding with `emission`
  void Step(int position, const HMMModel::EmissionArray *emission);

  // Stores result into `part_of_speech_tag_instance`
  void StoreResult(PartOfSpeechTagInstance *part_of_speech_tag_instance);

  // Gets the emission of word at `position` of `term_instance_`
  const HMMModel::EmissionArray *EmissionAt(int position);

  DISALLOW_COPY_AND_ASSIGN(HMMPartOfSpeechTagger);
};

}  // namespace milkcat

#endif  // SRC_TAGGER_HMM_PART_OF_SPEECH_TAGGER_H_
