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
// model_factory.h --- Created at 2014-02-03
// libmilkcat.h --- Created at 2014-02-06
// model_factory.h --- Created at 2014-04-02
// model_impl.h --- Created at 2014-09-02
//

#include <string>
#include <vector>
#include "parser/dependency_parser.h"
#include "util/mutex.h"
#include "util/util.h"

#ifndef SRC_COMMON_MODEL_FACTORY_H_
#define SRC_COMMON_MODEL_FACTORY_H_

namespace milkcat {

class PerceptronModel;
template <class T> class StaticArray;
template <class K, class V> class StaticHashTable;
class CRFModel;
class HMMModel;
class ReimuTrie;

// A factory class that can obtain any model data class needed by MilkCat
// in singleton mode. All the GetXX fucnctions are thread safe
class Model {
 public:
  explicit Model(const char *model_dir_path);
  ~Model();

  // Get the index for word which were used in unigram cost, bigram cost
  // hmm pos model and oov property
  const ReimuTrie *Index(Status *status);

  // Sets the user dictionary for the segmenter
  bool SetUserDictionary(const char *path);

  // If the model has loaded the user dictionary
  bool HasUserDictionary() const { return user_index_ != NULL; }

  const ReimuTrie *UserIndex(Status *status);
  const StaticArray<float> *UserCost(Status *status);

  const StaticArray<float> *UnigramCost(Status *status);
  const StaticHashTable<int64_t, float> *BigramCost(Status *status);

  // Get the CRF word segmenter model
  const CRFModel *CRFSegModel(Status *status);

  // Get the CRF word part-of-speech model
  const CRFModel *CRFPosModel(Status *status);

  // Get the HMM word part-of-speech model
  const HMMModel *HMMPosModel(Status *status);

  // Get the character's property in out-of-vocabulary word recognition
  const ReimuTrie *OOVProperty(Status *status);

  // Get the dependency model
  PerceptronModel *YamadaModel(Status *status);
  PerceptronModel *BeamYamadaModel(Status *status);

  // Get the feature template for dependency parsing
  DependencyParser::FeatureTemplate *DependencyTemplate(Status *status);

  // Reads user dictionary `userdict_path` for word segmenter
  void ReadUserDictionary(const char *userdict_path, Status *status);

 private:
  std::string model_dir_;
  Mutex mutex;

  const ReimuTrie *unigram_index_;
  const ReimuTrie *user_index_;
  const StaticArray<float> *unigram_cost_;
  const StaticArray<float> *user_cost_;
  const StaticHashTable<int64_t, float> *bigram_cost_;
  const CRFModel *seg_model_;
  const CRFModel *crf_pos_model_;
  const HMMModel *hmm_pos_model_;
  const ReimuTrie *oov_property_;
  PerceptronModel *dependency_;
  DependencyParser::FeatureTemplate *dependency_feature_;
};

}  // namespace milkcat

#endif  // SRC_COMMON_MODEL_FACTORY_H_