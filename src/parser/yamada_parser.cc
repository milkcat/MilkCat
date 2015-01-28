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
// dependency_parser.h
// naive_arceager_dependency_parser.h --- Created at 2013-08-10
// yamada_parser.cc --- Created at 2015-01-27
//

#include "parser/yamada_parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <set>
#include <vector>
#include "ml/feature_set.h"
#include "ml/perceptron.h"
#include "ml/perceptron_model.h"
#include "segmenter/term_instance.h"
#include "parser/feature_template.h"
#include "parser/node.h"
#include "parser/orcale.h"
#include "parser/state.h"
#include "parser/tree_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "util/pool.h"
#include "util/readable_file.h"
#include "util/status.h"
#include "util/util.h"

namespace milkcat {

YamadaParser::YamadaParser(
    PerceptronModel *perceptron_model,
    FeatureTemplate *feature): DependencyParser(perceptron_model, feature) {
  state_ = new State();
}

YamadaParser::~YamadaParser() {
  delete state_;
  state_ = NULL;
}

YamadaParser *YamadaParser::New(Model::Impl *model_impl, Status *status) {
  PerceptronModel *perceptron_model = model_impl->DependencyModel(status);

  FeatureTemplate *feature_template = NULL;
  if (status->ok()) feature_template = model_impl->DependencyTemplate(status);

  YamadaParser *self = NULL;

  if (status->ok()) {
    self = new YamadaParser(perceptron_model, feature_template);
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }  
}

// Compare the yid by cost in `perceptron`
class IdxCostPairCmp {
 public:
  IdxCostPairCmp(Perceptron *perceptron): perceptron_(perceptron) {
  }
  bool operator()(int y1, int y2) {
    return perceptron_->ycost(y1) < perceptron_->ycost(y2);
  }
 private:
  Perceptron *perceptron_;
};

int YamadaParser::Next() {  
  int feature_num = feature_->Extract(state_,
                                      term_instance_,
                                      part_of_speech_tag_instance_,
                                      feature_set_);
  int yid = perceptron_->Classify(feature_set_);

  // If the first candidate action is not allowed
  if (Allow(state_, yid) == false) {
    // sort the results by its cost
    std::vector<int> idheap(perceptron_->ysize());
    for (int i = 0; i < perceptron_->ysize(); ++i) idheap[i] = i;

    // Comparetor of index
    IdxCostPairCmp comp(perceptron_);
    std::make_heap(idheap.begin(), idheap.end(), comp);
    do {
      // Unshift transition
      if (idheap.size() == 0) return -1;

      std::pop_heap(idheap.begin(), idheap.end(), comp);
      yid = idheap.back();
      idheap.pop_back();
    } while (Allow(state_, yid) == false);
  }

  // puts(perceptron_->yname(yid));
  return yid;
}

void YamadaParser::StoreResult(
    TreeInstance *tree_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  StoreStateIntoInstance(state_, tree_instance);
}

void YamadaParser::Step(int yid) {
  StateMove(state_, yid);
}

// Do some preparing work
void YamadaParser::Start(
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  term_instance_ = term_instance;
  part_of_speech_tag_instance_ = part_of_speech_tag_instance;
  
  node_pool_->ReleaseAll();
  state_->Initialize(node_pool_, term_instance->size());  
}

void YamadaParser::Parse(
    TreeInstance *tree_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  Start(term_instance, part_of_speech_tag_instance);

  while (!(state_->InputEnd() && state_->StackOnlyOneElement())) {
    int yid = Next();
    Step(yid);
  }
  
  StoreResult(tree_instance,
              term_instance,
              part_of_speech_tag_instance);
}

}  // namespace milkcat
