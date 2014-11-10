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
//

#define DEBUG

#include "parser/naive_arceager_dependency_parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <set>
#include <vector>
#include "ml/feature_set.h"
#include "ml/averaged_multiclass_perceptron.h"
#include "ml/multiclass_perceptron.h"
#include "ml/multiclass_perceptron_model.h"
#include "segmenter/term_instance.h"
#include "parser/dependency_instance.h"
#include "parser/feature_template.h"
#include "parser/node.h"
#include "parser/orcale.h"
#include "parser/state.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/pool.h"
#include "utils/readable_file.h"
#include "utils/status.h"
#include "utils/utils.h"

namespace milkcat {

NaiveArceagerDependencyParser::NaiveArceagerDependencyParser(
    MulticlassPerceptronModel *perceptron_model,
    FeatureTemplate *feature): DependencyParser(perceptron_model, feature) {
  state_ = new State();
}

NaiveArceagerDependencyParser::~NaiveArceagerDependencyParser() {
  delete state_;
  state_ = NULL;
}

NaiveArceagerDependencyParser *
NaiveArceagerDependencyParser::New(Model::Impl *model_impl,
                                   Status *status) {
  MulticlassPerceptronModel *
  perceptron_model = model_impl->DependencyModel(status);

  FeatureTemplate *feature_template = NULL;
  if (status->ok()) feature_template = model_impl->DependencyTemplate(status);

  NaiveArceagerDependencyParser *self = NULL;

  if (status->ok()) {
    self = new NaiveArceagerDependencyParser(perceptron_model,
                                             feature_template);
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
  IdxCostPairCmp(MulticlassPerceptron *perceptron): 
      perceptron_(perceptron) {
  }
  bool operator()(int y1, int y2) {
    return perceptron_->ycost(y1) < perceptron_->ycost(y2);
  }
 private:
  MulticlassPerceptron *perceptron_;
};

int NaiveArceagerDependencyParser::Next() {  
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

void NaiveArceagerDependencyParser::StoreResult(
    DependencyInstance *dependency_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  StoreStateIntoInstance(state_, dependency_instance);
}

void NaiveArceagerDependencyParser::Step(int yid) {
  StateStep(state_, yid);
}


// Do some preparing work
void NaiveArceagerDependencyParser::StartParse(
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  term_instance_ = term_instance;
  part_of_speech_tag_instance_ = part_of_speech_tag_instance;
  
  node_pool_->ReleaseAll();
  state_->Initialize(node_pool_, term_instance->size());  
}

void NaiveArceagerDependencyParser::Parse(
    DependencyInstance *dependency_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  StartParse(term_instance, part_of_speech_tag_instance);

  while (!(state_->InputEnd() && state_->StackOnlyOneElement())) {
    int yid = Next();
    Step(yid);
  }
  
  StoreResult(dependency_instance,
              term_instance,
              part_of_speech_tag_instance);
}

void NaiveArceagerDependencyParser::Train(
    const char *training_corpus,
    const char *template_filename,
    const char *model_prefix,
    int max_iteration,
    Status *status) {
  // Some instances
  TermInstance *term_instance = new TermInstance();
  PartOfSpeechTagInstance *tag_instance = new PartOfSpeechTagInstance();
  DependencyInstance *dependency_instance_correct = new DependencyInstance();
  DependencyInstance *dependency_instance = new DependencyInstance();

  // Orcale to predict correct transitions
  Orcale *orcale = new Orcale();

  // Gets the label name of transitions
  ReadableFile *fd;
  std::set<std::string> yname_set;
  const char *label = NULL;
  if (status->ok()) fd = ReadableFile::New(training_corpus, status);
  while (status->ok() && !fd->Eof()) {
    LoadDependencyTreeInstance(
        fd,
        term_instance,
        tag_instance,
        dependency_instance,
        status);
    if (status->ok()) {
      orcale->Parse(dependency_instance);
      while ((label = orcale->Next()) != NULL) {
        yname_set.insert(label);
      }
    }
  }
  delete fd;
  fd = NULL;

  // Read template file
  FeatureTemplate *feature = NULL;
  if (status->ok()) feature = FeatureTemplate::Open(template_filename, status);

  // Creates perceptron model, percpetron and the parser
  NaiveArceagerDependencyParser *parser = NULL;
  MulticlassPerceptronModel *model = NULL;
  MulticlassPerceptron *percpetron = NULL;
  if (status->ok()) {
    std::vector<std::string> yname(yname_set.begin(), yname_set.end());
    model = new MulticlassPerceptronModel(yname);
    parser = new NaiveArceagerDependencyParser(model, feature);
    percpetron = new AveragedMulticlassPerceptron(model);
  }

  // Start training
  for (int iter = 0; iter < max_iteration && status->ok(); ++iter) {
    const char *label;
    int yid;
    int correct = 0, total = 0;
    if (status->ok()) fd = ReadableFile::New(training_corpus, status);
    while (status->ok() && !fd->Eof()) {
      LoadDependencyTreeInstance(
          fd,
          term_instance,
          tag_instance,
          dependency_instance_correct,
          status);
      if (status->ok()) {
        orcale->Parse(dependency_instance_correct);
        parser->StartParse(term_instance, tag_instance);
        while ((label = orcale->Next()) != NULL) {
          yid = model->yid(label);
          ASSERT(yid != MulticlassPerceptronModel::kIdNone, "Unexpected label");

          // Generates feature set
          parser->Next();
          if (percpetron->Train(parser->feature_set_, label)) {
            ++correct;
          }
          parser->Step(yid);
          ++total;
        }

#ifdef DEBUG
        parser->StoreResult(dependency_instance, term_instance, tag_instance);
        // Check whether the orcale is correct
        for (int i = 0; i < dependency_instance->size(); ++i) {
          ASSERT(dependency_instance->head_node_at(i) == 
                 dependency_instance_correct->head_node_at(i),
                 "Orcale failed");
        }
#endif
      }
    }
    delete fd;
    fd = NULL;
    
    printf("Iter %d: p = %.3f\n",
           iter + 1,
           static_cast<float>(correct) / total);
  }

  if (status->ok()) {
    percpetron->FinishTrain();
    model->Save(model_prefix, status);
  }
  if (!status->ok()) puts(status->what());

  delete term_instance;
  delete tag_instance;
  delete dependency_instance_correct;
  delete dependency_instance;
  delete orcale;
  delete model;
  delete parser;
  delete percpetron;
}

}  // namespace milkcat
