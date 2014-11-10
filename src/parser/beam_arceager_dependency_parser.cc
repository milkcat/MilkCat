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
// beam_arceager_dependency_parser.cc --- Created at 2014-10-31
//

// #define DEBUG

#include "parser/beam_arceager_dependency_parser.h"

#include <algorithm>
#include <set>
#include "common/model_impl.h"
#include "ml/averaged_multiclass_perceptron.h"
#include "ml/multiclass_perceptron.h"
#include "ml/multiclass_perceptron_model.h"
#include "parser/dependency_instance.h"
#include "parser/feature_template.h"
#include "parser/node.h"
#include "parser/orcale.h"
#include "parser/state.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/pool.h"
#include "utils/readable_file.h"
#include "utils/utils.h"

namespace milkcat {

BeamArceagerDependencyParser::BeamArceagerDependencyParser(
    MulticlassPerceptronModel *perceptron_model,
    FeatureTemplate *feature):
        DependencyParser(perceptron_model, feature) {
  state_pool_ = new Pool<State>();
  agent_ = new float[perceptron_model->ysize() * kBeamSize];
  beam_ = new State *[kBeamSize];
  next_beam_ = new State *[kBeamSize];
  beam_size_ = 0;
  agent_size_ = 0;
}

BeamArceagerDependencyParser::~BeamArceagerDependencyParser() {
  delete state_pool_;
  state_pool_ = NULL;

  delete[] beam_;
  beam_ = NULL;

  delete[] next_beam_;
  next_beam_ = NULL;

  delete[] agent_;
  agent_ = NULL;
}

BeamArceagerDependencyParser *
BeamArceagerDependencyParser::New(Model::Impl *model,
                                  Status *status) {
  MulticlassPerceptronModel *
  perceptron_model = model->DependencyModel(status);

  FeatureTemplate *feature_template = NULL;
  if (status->ok()) feature_template = model->DependencyTemplate(status);

  BeamArceagerDependencyParser *self = NULL;

  if (status->ok()) {
    self = new BeamArceagerDependencyParser(perceptron_model,
                                             feature_template);
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }  
}

// Compare the yid by the 
class CompareIdxByCostInArray {
 public:
  CompareIdxByCostInArray(float *array, int size): 
      array_(array), array_size_(size) {
  }
  bool operator()(int idx1, int idx2) {
    ASSERT(idx1 < array_size_ && idx2 < array_size_, "Idx overflow");
    return array_[idx1] > array_[idx2];
  }
 private:
  const float *array_;
  int array_size_;
};

// Start to parse the sentence
void BeamArceagerDependencyParser::StartParse(
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  state_pool_->ReleaseAll();
  node_pool_->ReleaseAll();

  term_instance_ = term_instance;
  part_of_speech_tag_instance_ = part_of_speech_tag_instance;

  // Push root state into `beam_`
  State *root_state = state_pool_->Alloc();
  root_state->Initialize(node_pool_, term_instance->size());
  beam_[0] = root_state;
  beam_size_ = 1;
}

bool BeamArceagerDependencyParser::Step() {
  // Calculate the cost of transitions in each state of `beam_`, store them into
  // `agent_` 
  int transition_num = perceptron_->ysize();
  for (int i = 0; i < beam_size_; ++i) {
    int feature_num = feature_->Extract(beam_[i],
                                        term_instance_,
                                        part_of_speech_tag_instance_,
                                        feature_set_);
    int yid = perceptron_->Classify(feature_set_);
    for (int yid = 0; yid < transition_num; ++yid) {
      agent_[i * transition_num + yid] = perceptron_->ycost(yid) + 
                                         beam_[i]->weight();
    }
  }
  agent_size_ = beam_size_ * transition_num;

  // Partial sorts the agent to get the N-best transitions (N = kBeamSize)
  CompareIdxByCostInArray cmp(agent_, agent_size_);
  std::vector<int> idx_heap;
  for (int i = 0; i < agent_size_; ++i) {
    int yid = i % transition_num;
    int state_idx = i / transition_num;
    if (Allow(beam_[state_idx], yid)) {
      // If state allows transition `yid`, stores them into `idx_heap`
      if (idx_heap.size() < kBeamSize) {
        idx_heap.push_back(i);
        std::push_heap(idx_heap.begin(), idx_heap.end(), cmp);
      } else if (cmp(idx_heap[0], i) == false) {
        // agent_[idx_heap[0]] < agent_[i]
        std::pop_heap(idx_heap.begin(), idx_heap.end(), cmp);
        idx_heap.back() = i;
        std::push_heap(idx_heap.begin(), idx_heap.end(), cmp);
      }
    }
  }

  // Create new states into `next_beam_` from `idx_heap`
  for (std::vector<int>::iterator
       it = idx_heap.begin(); it != idx_heap.end(); ++it) {
    int yid = *it % transition_num;
    int state_idx = *it / transition_num;

    // Copy the statue from beam
    State *state = state_pool_->Alloc();
    beam_[state_idx]->CopyTo(state);

    // Make a transition `yid`
    StateStep(state, yid);
    state->set_weight(agent_[*it]);
    state->set_previous(beam_[state_idx]);

    int to_idx = it - idx_heap.begin();
    next_beam_[to_idx] = state;
  }

  // Swap beam_ and next_beam_
  State **t_beam = beam_;
  beam_ = next_beam_;
  next_beam_ = t_beam;

  beam_size_ = idx_heap.size();

  if (beam_size_ == 0)
    return false;
  else 
    return true;
}

inline bool StateCmp(const DependencyParser::State *s1,
                     const DependencyParser::State *s2) {
  return s1->weight() < s2->weight();
}

void BeamArceagerDependencyParser::StoreResult(
    DependencyInstance *dependency_instance) {
  State *max_state = *std::max_element(beam_, beam_ + beam_size_, StateCmp);
  StoreStateIntoInstance(max_state, dependency_instance);
}

void BeamArceagerDependencyParser::Parse(
    DependencyInstance *dependency_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  StartParse(term_instance, part_of_speech_tag_instance);
  int len = term_instance->size();
  for (int i = 0; i < 2 * len; ++i) {
    if (false == Step()) break;
  }
  // DumpBeam();
  StoreResult(dependency_instance);
}

void BeamArceagerDependencyParser::DumpBeam() {
  for (int beam_idx = 0; beam_idx < beam_size_; ++beam_idx) {
    State *state = beam_[beam_idx];
    std::vector<int> sequence;
    std::vector<bool> corr_sequence;
    while (state->previous() != NULL) {
      sequence.push_back(state->last_transition());
      corr_sequence.push_back(state->correct());
      state = state->previous();
    }
    printf("BEAM %d: ", beam_idx);
    while (!sequence.empty()) {
      printf("%s(%c)  ",
             perceptron_->yname(sequence.back()),
             corr_sequence.back()? 'T': 'F');
      sequence.pop_back();
      corr_sequence.pop_back();
    }
    printf(", weight = %f\n", beam_[beam_idx]->weight());
  }
}

DependencyParser::State *BeamArceagerDependencyParser::TrainState(
    DependencyParser::State *incorrect_state,
    DependencyParser::State *orcale_state,
    BeamArceagerDependencyParser *parser,
    MulticlassPerceptron *percpetron) {
  // Find the first incorrect state in the history of `incorrect_state`
  while (incorrect_state->correct() == false) {
    int correct_yid = orcale_state->last_transition();
    int incorrect_yid = incorrect_state->last_transition();
    incorrect_state = incorrect_state->previous();
    orcale_state = orcale_state->previous();

    parser->feature_->Extract(
        orcale_state,
        parser->term_instance_,
        parser->part_of_speech_tag_instance_,
        parser->feature_set_);
    percpetron->Update(parser->feature_set_, correct_yid, 1.0f);

    parser->feature_->Extract(
        incorrect_state,
        parser->term_instance_,
        parser->part_of_speech_tag_instance_,
        parser->feature_set_);
    percpetron->Update(parser->feature_set_, incorrect_yid, -1.0f);
  }

  return orcale_state;
}

void BeamArceagerDependencyParser::Train(
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
  int total_instance = 0;
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
    ++total_instance;
  }
  delete fd;
  fd = NULL;

  // Read template file
  FeatureTemplate *feature = NULL;
  if (status->ok()) feature = FeatureTemplate::Open(template_filename, status);

  // Creates perceptron model, percpetron and the parser
  BeamArceagerDependencyParser *parser = NULL;
  MulticlassPerceptronModel *model = NULL;
  MulticlassPerceptron *perceptron = NULL;
  if (status->ok()) {
    std::vector<std::string> yname(yname_set.begin(), yname_set.end());
    model = new MulticlassPerceptronModel(yname);
    parser = new BeamArceagerDependencyParser(model, feature);
    perceptron = new AveragedMulticlassPerceptron(model);
  }

  // Start training
  std::vector<int> correct_sequence;
  State *orcale_state = NULL;
  for (int iter = 0; iter < max_iteration && status->ok(); ++iter) {
    const char *label;
    int yid;
    int error = 0;
    if (status->ok()) fd = ReadableFile::New(training_corpus, status);
    int instance_num = 0;
    while (status->ok() && !fd->Eof()) {
      correct_sequence.clear();
      LoadDependencyTreeInstance(
          fd,
          term_instance,
          tag_instance,
          dependency_instance_correct,
          status);
      ++instance_num;
      for (int i = 0; i < term_instance->size(); ++i) {
        LOG("%s %s %d %s\n", 
               term_instance->term_text_at(i),
               tag_instance->part_of_speech_tag_at(i),
               dependency_instance_correct->head_node_at(i),
               dependency_instance_correct->dependency_type_at(i)); 
      }
      if (status->ok()) {
        orcale->Parse(dependency_instance_correct);
        parser->StartParse(term_instance, tag_instance);
        orcale_state = parser->state_pool_->Alloc();
        orcale_state->Initialize(parser->node_pool_, term_instance->size());
        while ((label = orcale->Next()) != NULL) {
          yid = model->yid(label);

          // Move `orcale_state` according to `orcale`
          State *s = parser->state_pool_->Alloc();
          orcale_state->CopyTo(s);
          parser->StateStep(s, yid);
          s->set_previous(orcale_state);
          orcale_state = s;
          ASSERT(yid != MulticlassPerceptronModel::kIdNone, "Unexpected label");

          // Marks correct = true for the correct state
          parser->Step();
          bool correct = false;
          for (int i = 0; i < parser->beam_size_; ++i) {
            if (parser->beam_[i]->last_transition() == yid &&
                parser->beam_[i]->previous()->correct() == true) {
              parser->beam_[i]->set_correct(true);
              correct = true;
            } else {
              parser->beam_[i]->set_correct(false);
            }
          }
#ifdef DEBUG
          parser->DumpBeam();
#endif
          perceptron->IncCount();
          // If no state in the beam is correct
          if (correct == false) {
            
            // Update model
            State *max_state = *std::max_element(
                parser->beam_,
                parser->beam_ + parser->beam_size_,
                StateCmp);
            TrainState(max_state, orcale_state, parser, perceptron);

            // Push the correct status into beam
            State *s = parser->state_pool_->Alloc();
            orcale_state->CopyTo(s);
            parser->beam_[0] = s;
            parser->beam_size_ = 1;

            error++;
          }  // if (correct == false)
        }  // while ((label = orcale->Next()) != NULL)

        State *max_state = *std::max_element(
            parser->beam_,
            parser->beam_ + parser->beam_size_,
            StateCmp);
        if (max_state->correct() == false) {
          TrainState(max_state, orcale_state, parser, perceptron);
          error++;
        }   

      }  // End if

      if (instance_num % 100 == 1) {
        printf("\rIter %d, %.2f%%, err = %d",
               iter + 1,
               100.0f * instance_num / total_instance,
               error);
        fflush(stdout);
      }
    }
    delete fd;
    fd = NULL;
    
    printf("\rIter %d, err = %d, OK            \n", iter + 1, error);
  }

  if (status->ok()) {
    perceptron->FinishTrain();
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
  delete perceptron;
}

}  // namespace milkcat
