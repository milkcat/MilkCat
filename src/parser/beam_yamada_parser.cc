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
// beam_yamada_parser.cc --- Created at 2015-01-27
//

// #define DEBUG

#include "parser/beam_yamada_parser.h"

#include <algorithm>
#include <map>
#include <set>
#include "common/model_impl.h"
#include "common/reimu_trie.h"
#include "common/static_array.h"
#include "ml/feature_set.h"
#include "ml/perceptron.h"
#include "ml/perceptron_model.h"
#include "parser/feature_template.h"
#include "parser/feature_template-inl.h"
#include "parser/node.h"
#include "parser/orcale.h"
#include "parser/state.h"
#include "parser/tree_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "util/pool.h"
#include "util/readable_file.h"
#include "util/status.h"
#include "util/string_builder.h"
#include "util/util.h"

namespace milkcat {

class BeamYamadaParser::StateCmp {
 public:
  // Since in `Beam` the least is the `Best`. So use `>` instead of '<'
  bool operator()(const State *s1, const State *s2) {
    return s1->weight() > s2->weight();
  }
};

BeamYamadaParser::BeamYamadaParser(
    PerceptronModel *perceptron_model,
    FeatureTemplate *feature,
    int beam_size): DependencyParser(perceptron_model, feature) {
  state_pool_ = new Pool<State>();
  agent_ = new float[perceptron_model->ysize() * beam_size];
  beam_ = new Beam<State, StateCmp>(beam_size);
  next_beam_ = new Beam<State, StateCmp>(beam_size);
  agent_size_ = 0;
  beam_size_ = beam_size;
}

BeamYamadaParser::~BeamYamadaParser() {
  delete state_pool_;
  state_pool_ = NULL;

  delete beam_;
  beam_ = NULL;

  delete next_beam_;
  next_beam_ = NULL;

  delete[] agent_;
  agent_ = NULL;
}

BeamYamadaParser *
BeamYamadaParser::New(Model::Impl *model, Status *status) {
  PerceptronModel *perceptron_model = model->BeamYamadaModel(status);

  FeatureTemplate *feature_template = NULL;
  if (status->ok()) feature_template = model->DependencyTemplate(status);

  BeamYamadaParser *self = NULL;
  if (status->ok()) {
    self = new BeamYamadaParser(perceptron_model,
                                feature_template,
                                kParserBeamSize);
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
    MC_ASSERT(idx1 < array_size_ && idx2 < array_size_, "idx overflow");
    // To make a minimum heap, use `>` instead '<'
    return array_[idx1] > array_[idx2];
  }
 private:
  const float *array_;
  int array_size_;
};

// Start to parse the sentence
void BeamYamadaParser::Start(
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  state_pool_->ReleaseAll();
  node_pool_->ReleaseAll();

  term_instance_ = term_instance;
  part_of_speech_tag_instance_ = part_of_speech_tag_instance;

  // Push root state into `beam_`
  State *root_state = state_pool_->Alloc();
  root_state->Initialize(node_pool_, term_instance->size());
  beam_->Clear();
  next_beam_->Clear();
  beam_->Add(root_state);
}

BeamYamadaParser::State *
BeamYamadaParser::StateCopyAndMove(State *state, int yid) {
  State *next_state = state_pool_->Alloc();
  state->CopyTo(next_state);

  // Make a transition `yid`
  StateMove(next_state, yid);
  next_state->set_previous(state);

  return next_state;
}

bool BeamYamadaParser::Step() {
  // Calculate the cost of transitions in each state of `beam_`, store them into
  // `agent_` 
  int ysize = perceptron_->ysize();
  for (int beam_idx = 0; beam_idx < beam_->size(); ++beam_idx) {
    ExtractFeatureFromState(beam_->at(beam_idx), feature_set_);
    int yid = perceptron_->Classify(feature_set_);
    
    for (int yid = 0; yid < ysize; ++yid) {
      agent_[beam_idx * ysize + yid] = static_cast<float>(
          perceptron_->ycost(yid) + beam_->at(beam_idx)->weight());
    }
  }
  agent_size_ = beam_->size() * ysize;
  
  // Partial sorts the agent to get the N-best transitions (N = beam_size_)
  CompareIdxByCostInArray cmp(agent_, agent_size_);
  std::vector<int> idx_heap;
  for (int agent_idx = 0; agent_idx < agent_size_; ++agent_idx) {
    int yid = agent_idx % ysize;
    int beam_idx = agent_idx / ysize;
    if (Allow(beam_->at(beam_idx), yid)) {
      // If state allows transition `yid`, stores them into `idx_heap`
      if (static_cast<int>(idx_heap.size()) < beam_size_) {
        idx_heap.push_back(agent_idx);
        std::push_heap(idx_heap.begin(), idx_heap.end(), cmp);
      } else if (cmp(idx_heap[0], agent_idx) == false) {
        // agent_[idx_heap[0]] < agent_[i]
        std::pop_heap(idx_heap.begin(), idx_heap.end(), cmp);
        idx_heap.back() = agent_idx;
        std::push_heap(idx_heap.begin(), idx_heap.end(), cmp);
      }
    }
  }


  // Create new states into `next_beam_` from `idx_heap`
  next_beam_->Clear();
  for (std::vector<int>::iterator
       it = idx_heap.begin(); it != idx_heap.end(); ++it) {
    int yid = *it % ysize;
    int beam_idx = *it / ysize;

    // Copy the statue from beam
    State *state = StateCopyAndMove(beam_->at(beam_idx), yid);
    state->set_weight(agent_[*it]);
    next_beam_->Add(state);
  }


  // If no transition could perform, just remains `beam_` to the last state and
  // return false
  if (next_beam_->size() == 0) return false;

  // Swap beam_ and next_beam_
  Beam<State, StateCmp> *t_beam = beam_;
  beam_ = next_beam_;
  next_beam_ = t_beam;

  return true;
}

void BeamYamadaParser::StoreResult(
    TreeInstance *tree_instance) {
  State *max_state = beam_->Best();
  StoreStateIntoInstance(max_state, tree_instance);
}

void BeamYamadaParser::Parse(
    TreeInstance *tree_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  Start(term_instance, part_of_speech_tag_instance);
  int len = term_instance->size();
  while (false != Step());
  StoreResult(tree_instance);
}

void BeamYamadaParser::ExtractFeatureFromState(
    const State* state,
    FeatureSet *feature_set) {
  int feature_num = feature_->Extract(state,
                                      term_instance_,
                                      part_of_speech_tag_instance_,
                                      feature_set);
}

// Training `perceptron` with an correct (orcale) and incorrect state pair,
void BeamYamadaParser::UpdateWeightForState(
    DependencyParser::State *incorrect_state,
    DependencyParser::State *correct_state,
    BeamYamadaParser *parser,
    Perceptron *percpetron) {
  // Find the first incorrect state in the history of `incorrect_state`
  while (incorrect_state->correct() == false) {
    int correct_yid = correct_state->last_transition();
    int incorrect_yid = incorrect_state->last_transition();
    incorrect_state = incorrect_state->previous();
    correct_state = correct_state->previous();
    
    parser->ExtractFeatureFromState(correct_state, parser->feature_set_);
    percpetron->Update(parser->feature_set_, correct_yid, 1.0f);

    parser->ExtractFeatureFromState(incorrect_state, parser->feature_set_);
    percpetron->Update(parser->feature_set_, incorrect_yid, -1.0f);
  }
}

// Inserts `key` into `reimu_trie` and returns the id of `key`. If the key
// already exists, it do nothing, if the key didn't exist, inserts (key, *count)
// pair and increases count by 1.
int InsertIntoIndex(ReimuTrie *reimu_trie, const char *key, int *count) {
  int id = reimu_trie->Get(key, -1);
  if (id < 0) {
    reimu_trie->Put(key, *count);
    id = *count;
    ++*count;
  }
  return id;
}

void BeamYamadaParser::Train(
    const char *training_corpus,
    const char *template_filename,
    const char *model_prefix,
    int beam_size,
    int max_iteration,
    Status *status) {
  // Some instances
  TermInstance *term_instance = new TermInstance();
  PartOfSpeechTagInstance *tag_instance = new PartOfSpeechTagInstance();
  TreeInstance *tree_instance_correct = new TreeInstance();
  TreeInstance *tree_instance = new TreeInstance();

  // Orcale to predict correct transitions
  Orcale *orcale = new Orcale();

  // Gets the label name of transitions
  ReadableFile *fd = NULL;
  std::set<std::string> yname_set;
  const char *label = NULL;
  if (status->ok()) fd = ReadableFile::New(training_corpus, status);
  int total_instance = 0;
  while (status->ok() && !fd->Eof()) {
    LoadDependencyTreeInstance(
        fd,
        term_instance,
        tag_instance,
        tree_instance,
        status);
    if (status->ok()) {
      orcale->Parse(tree_instance);
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
  BeamYamadaParser *parser = NULL;
  PerceptronModel *model = NULL;
  Perceptron *perceptron = NULL;
  if (status->ok()) {
    std::vector<std::string> yname(yname_set.begin(), yname_set.end());
    model = new PerceptronModel(yname);
    parser = new BeamYamadaParser(model, feature, beam_size);
    perceptron = new Perceptron(model);
  }


  // Start training 
  State *correct_state = NULL;
  for (int iter = 0; iter < max_iteration && status->ok(); ++iter) {
    const char *label;
    int yid;
    int error = 0;
    if (status->ok()) fd = ReadableFile::New(training_corpus, status);
    int instance_num = 0;
    while (status->ok() && !fd->Eof()) {
      LoadDependencyTreeInstance(
          fd,
          term_instance,
          tag_instance,
          tree_instance_correct,
          status);
      ++instance_num;
      if (status->ok()) {
        orcale->Parse(tree_instance_correct);
        parser->Start(term_instance, tag_instance);

        // `correct_state` always stores the current correct state from orcale
        correct_state = parser->state_pool_->Alloc();
        correct_state->Initialize(parser->node_pool_, term_instance->size());
        while ((label = orcale->Next()) != NULL) {
          yid = model->yid(label);

          // Move `correct_state` according to `orcale`
          correct_state = parser->StateCopyAndMove(correct_state, yid);
          MC_ASSERT(yid != PerceptronModel::kIdNone, "unexpected label");

          // Marks `correct = true` for the correct state in beam_
          parser->Step();
          bool correct = false;
          for (int beam_idx = 0; beam_idx < parser->beam_->size(); ++beam_idx) {
            if (parser->beam_->at(beam_idx)->last_transition() == yid &&
                parser->beam_->at(beam_idx)->previous()->correct() == true) {
              // If previous state is correct and current transition is correct
              parser->beam_->at(beam_idx)->set_correct(true);
              correct = true;
            } else {
              parser->beam_->at(beam_idx)->set_correct(false);
            }
          }

          perceptron->IncreaseSampleCount();
          // If no state in the beam is correct
          if (correct == false) {
            
            // Update model
            State *max_state = parser->beam_->Best();
            UpdateWeightForState(max_state, correct_state, parser, perceptron);

            // Push the correct status into beam
            State *state = parser->state_pool_->Alloc();
            correct_state->CopyTo(state);
            parser->beam_->Clear();
            parser->beam_->Add(state);

            error++;
          }  // if (correct == false)
        }  // while ((label = orcale->Next()) != NULL)

        State *max_state = parser->beam_->Best();
        if (max_state->correct() == false) {
          UpdateWeightForState(max_state, correct_state, parser, perceptron);
          error++;
        }   

        if (instance_num % 100 == 0) {
          printf("\r#%d, %.2f%%, err = %d",
                 iter + 1,
                 100.0f * instance_num / total_instance,
                 error);
          fflush(stdout);
        }
      }  // if (statuc->ok())
    }  // while (status->ok() && !fd->Eof())
    delete fd;
    fd = NULL;

    putchar('\r');
    for (int _ = 0; _ < 64; ++_) putchar(' ');
    printf("\r#%d, err = %d, OK\n", iter + 1, error);
  }

  if (status->ok()) {
    perceptron->FinishTrain();
    model->Save(model_prefix, status);
  }

  if (!status->ok()) puts(status->what());

  delete term_instance;
  delete tag_instance;
  delete tree_instance_correct;
  delete tree_instance;
  delete orcale;
  delete model;
  delete parser;
  delete perceptron;
}

}  // namespace milkcat
