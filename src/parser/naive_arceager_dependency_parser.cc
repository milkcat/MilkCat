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
// dependency_parser.h --- Created at 2013-08-10
//

#include "parser/naive_arceager_dependency_parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include "ml/maxent_classifier.h"
#include "segmenter/term_instance.h"
#include "parser/dependency_feature.h"
#include "parser/dependency_instance.h"
#include "parser/dependency_node.h"
#include "parser/dependency_state.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/pool.h"
#include "utils/status.h"
#include "utils/utils.h"

namespace milkcat {


NaiveArceagerDependencyParser::NaiveArceagerDependencyParser():
    maxent_classifier_(NULL),
    state_(NULL),
    feature_(NULL),
    node_pool_(NULL) {
}

NaiveArceagerDependencyParser::~NaiveArceagerDependencyParser() {
  delete maxent_classifier_;
  maxent_classifier_ = NULL;

  delete state_;
  state_ = NULL;

  delete feature_;
  feature_ = NULL;

  delete node_pool_;
  node_pool_ = NULL;
}

NaiveArceagerDependencyParser *
NaiveArceagerDependencyParser::New(Model::Impl *model_impl,
                                   Status *status) {
  const std::vector<std::string> *feature_templates = NULL;
  NaiveArceagerDependencyParser *self = new NaiveArceagerDependencyParser();
  self->state_ = new State();
  self->node_pool_ = new utils::Pool<Node>();

  const MaxentModel *maxent_model = model_impl->DependencyModel(status);

  if (status->ok()) {
    self->maxent_classifier_ = new MaxentClassifier(maxent_model);
    feature_templates = model_impl->DependencyTemplate(status);
  }

  if (status->ok()) self->feature_ = new Feature(feature_templates);

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }  
}

bool NaiveArceagerDependencyParser::AllowTransition(int label_id) const {
  const char *transition = maxent_classifier_->yname(label_id);
  if (strncmp(transition, "LARC", 4) == 0)
    return state_->AllowLeftArc();
  else if (strncmp(transition, "RARC", 4) == 0)
    return state_->AllowRightArc();
  else if (strncmp(transition, "REDU", 4) == 0)
    return state_->AllowReduce();
  else if (strncmp(transition, "SHIF", 4) == 0)
    return state_->AllowShift();
  
  ASSERT(false, "Illegal transition");
  return true;
}

int NaiveArceagerDependencyParser::NextTransition() {
  const char *feature_buffer[Feature::kFeatureMax];
  std::vector<std::pair<double, int> > 
  transition_costs(maxent_classifier_->ysize());

  int feature_num = feature_->BuildFeature(state_,
                                           term_instance_,
                                           part_of_speech_tag_instance_);
  for (int i = 0; i < feature_num; ++i)
    feature_buffer[i] = feature_->feature(i);

  int yid = maxent_classifier_->Classify(
      const_cast<const char **>(feature_buffer),
      feature_num);

  LOG("initial transition = ", maxent_classifier_->yname(yid));

  // If the first candidate action is not allowed
  if (AllowTransition(yid) == false) {
    // sort the results by its weight
    transition_costs.clear();
    for (int i = 0; i < maxent_classifier_->ysize(); ++i) {
      transition_costs.push_back(
          std::make_pair(maxent_classifier_->ycost(i), i));
    }
    std::sort(transition_costs.begin(), transition_costs.end());
    do {
      yid = transition_costs.back().second;
      transition_costs.pop_back();
      if (transition_costs.size() == 0) {
        yid = 0; // SHIFT
        break;
      }
    } while (AllowTransition(yid) == false);
  }

  LOG("final transition = ", maxent_classifier_->yname(yid));

  return yid;
}

void NaiveArceagerDependencyParser::StoreResult(
    DependencyInstance *dependency_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  for (int i = 0; i < state_->input_size() - 1; ++i) {
    // ignore the ROOT node
    const Node *node = state_->node_at(i + 1);
    dependency_instance->set_value_at(i, 
                                      node->dependency_label(),
                                      node->head_id());
  }

  dependency_instance->set_size(state_->input_size() - 1);
}

void NaiveArceagerDependencyParser::Parse(
    DependencyInstance *dependency_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  term_instance_ = term_instance;
  part_of_speech_tag_instance_ = part_of_speech_tag_instance;
  
  node_pool_->ReleaseAll();
  state_->Initialize(node_pool_, term_instance->size());

  while (!state_->InputEnd()) {
    int label_id = NextTransition();
    const char *label_str = maxent_classifier_->yname(label_id);
    if (strncmp("SHIF", label_str, 4) == 0) {
      state_->Shift();
    } else if (strncmp("REDU", label_str, 4) == 0) {
      state_->Reduce();
    } else if (strncmp("LARC", label_str, 4) == 0) {
      state_->LeftArc(label_str + 5);
    } else if (strncmp("RARC", label_str, 4) == 0) {
      state_->RightArc(label_str + 5);    
    } else {
      ASSERT(false, "Unknown transition");
    }
  }
  
  StoreResult(dependency_instance,
              term_instance,
              part_of_speech_tag_instance);
}

}  // namespace milkcat
