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
// hmm_part_of_speech_tagger.cc --- Created at 2013-11-10
//

#include "tagger/hmm_part_of_speech_tagger.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <string>
#include "libmilkcat.h"
#include "common/model_impl.h"
#include "common/reimu_trie.h"
#include "ml/beam.h"
#include "ml/hmm_model.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "util/pool.h"
#include "util/readable_file.h"
#include "util/util.h"

namespace milkcat {

struct HMMPartOfSpeechTagger::Node {
  int tag;
  double cost;
  const HMMPartOfSpeechTagger::Node *prevoius_node;

  inline void set_value(int tag, int cost, const Node *prevoius_node) {
    this->tag = tag;
    this->cost = cost;
    this->prevoius_node = prevoius_node;
  } 
};

// Compare two node potiners by its cost value
class HMMPartOfSpeechTagger::NodeComparator {
 public:
  bool operator()(HMMPartOfSpeechTagger::Node *n1, 
                  HMMPartOfSpeechTagger::Node *n2) {
    return n1->cost < n2->cost;
  }
};

HMMPartOfSpeechTagger::HMMPartOfSpeechTagger(): node_pool_(NULL),
                                                model_(NULL),
                                                PU_emission_(NULL),
                                                CD_emission_(NULL),
                                                NN_emission_(NULL),
                                                BOS_emission_(NULL),
                                                term_instance_(NULL) {
  node_pool_ = new Pool<Node>(); 

  for (int i = 0; i < kMaxBeams; ++i) {
    beams_[i] = new Beam<Node, NodeComparator>(kBeamSize);
  }
}

HMMPartOfSpeechTagger::~HMMPartOfSpeechTagger() {
  delete node_pool_;
  node_pool_ = NULL;

  delete PU_emission_;
  PU_emission_ = NULL;

  delete CD_emission_;
  CD_emission_ = NULL;

  delete NN_emission_;
  NN_emission_ = NULL;

  delete BOS_emission_;
  BOS_emission_ = NULL;

  for (int i = 0; i < kMaxBeams; ++i) {
    if (beams_[i] != NULL)
      delete beams_[i];
    beams_[i] = NULL;
  }
}

namespace {

// Creates an EmissionArray for tag specified by yname.
HMMModel::EmissionArray *NewEmission(
    const char *yname,
    const HMMModel *model,
    Status *status) {
  char error_message[1024];

  // Finds the yid for `yname`
  int yid;
  for (yid = 0; yid < model->ysize(); ++yid) {
    if (strcmp(yname, model->yname(yid)) == 0) break;
  }
  if (yid == model->ysize()) {
    sprintf(error_message, "Unable to file label '%s' from HMM model", yname);
    *status = Status::Corruption(error_message);
  } 

  HMMModel::EmissionArray *emission = NULL;
  if (status->ok()) {
    emission = new HMMModel::EmissionArray(1, 0);
    emission->set_yid_at(0, yid);
    emission->set_cost_at(0, 0.0f);
  }

  return emission;
}

}  // namespace

HMMPartOfSpeechTagger *HMMPartOfSpeechTagger::New(Model::Impl *model_factory,
                                                  Status *status) {
  const HMMModel *model = model_factory->HMMPosModel(status);
  if (status->ok()) {
    return New(model, status);
  } else {
    return NULL;
  }
}

HMMPartOfSpeechTagger *HMMPartOfSpeechTagger::New(
    const HMMModel *model,
    Status *status) {
  HMMPartOfSpeechTagger *self = new HMMPartOfSpeechTagger();
  self->model_ = model;

  if (status->ok()) {
    self->PU_emission_ = NewEmission("PU", self->model_, status);
  } 
  if (status->ok()) {
    self->NN_emission_ = NewEmission("NN", self->model_, status);
  }
  if (status->ok()) {
    self->CD_emission_ = NewEmission("CD", self->model_, status);
  }
  if (status->ok()) {
    self->BOS_emission_ = NewEmission("-BOS-", self->model_, status);
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

const HMMModel::EmissionArray *HMMPartOfSpeechTagger::EmissionAt(int position) {
  const HMMModel::EmissionArray *emission = NULL;
  emission = model_->Emission(term_instance_->term_text_at(position));

  if (emission == NULL) {
    int term_type = term_instance_->term_type_at(position);
    switch (term_type) {
      case Parser::kPunction:
      case Parser::kSymbol:
      case Parser::kOther:
        emission = PU_emission_;
        break;
      case Parser::kNumber:
        emission = CD_emission_;
        break;
      default:
        emission = NN_emission_;
        break;
    }
  }

  return emission;
}

inline void HMMPartOfSpeechTagger::StoreResult(
    PartOfSpeechTagInstance *tag_instance) {
  int beam_idx = term_instance_->size() + 1;

  // `beams_[beam_idx]` should have only one BOS node
  ASSERT(beams_[beam_idx]->size() == 1, "Last node in beam should be -BOS-");
  const Node *node = beams_[beam_idx]->at(0);

  int position = term_instance_->size() - 1;

  // Ignores the last BOS node
  node = node->prevoius_node;
  while (node->prevoius_node != NULL) {
    tag_instance->set_value_at(position, model_->yname(node->tag));
    position--;
    node = node->prevoius_node;
  }

  tag_instance->set_size(term_instance_->size());
}

void HMMPartOfSpeechTagger::Tag(
    PartOfSpeechTagInstance *part_of_speech_tag_instance,
    TermInstance *term_instance) {
  term_instance_ = term_instance;

  Node *begin_node = node_pool_->Alloc();
  begin_node->set_value(HMMModel::kBeginOfSenetnceId, 0, NULL);
  beams_[0]->Clear();
  beams_[0]->Add(begin_node);

  // Viterbi algorithm
  const HMMModel::EmissionArray *emission = NULL;
  for (int idx = 0; idx < term_instance->size(); ++idx) {
    emission = EmissionAt(idx);
    // beam_[0] is the BOS node, so use `idx + 1` for the word at `idx`
    Step(idx + 1, emission);
  }

  // The last BOS node
  emission = BOS_emission_;
  Step(term_instance->size() + 1, emission);

  // Save the result into `part_of_speech_tag_instance`
  StoreResult(part_of_speech_tag_instance);

  node_pool_->ReleaseAll();
}

void HMMPartOfSpeechTagger::Step(int position,
                                 const HMMModel::EmissionArray *emission) {
  Beam<Node, NodeComparator> *previous_beam = beams_[position - 1];
  Beam<Node, NodeComparator> *beam = beams_[position];

  previous_beam->Shrink();
  beam->Clear();

  for (int emission_idx = 0; emission_idx < emission->size(); ++emission_idx) {
    double min_cost = 1e38;
    const Node *min_node = NULL;
    int tag = emission->yid_at(emission_idx);
    double emission_cost = emission->cost_at(emission_idx);

    // To find the best path for current node
    for (int i = 0; i < previous_beam->size(); ++i) {
      const Node *left_node = previous_beam->at(i);
      int left_tag = left_node->tag;
      double transition_cost = model_->cost(left_tag, tag);
      double cost = left_node->cost + transition_cost + emission_cost;
      if (cost < min_cost) {
        min_cost = cost;
        min_node = left_node;
      }
      LOG("word = %s, left_tag = %s, tag = %s, emission_cost = %f, "
          "transition_cost = %f, total_cost = %f\n",
          position - 1 < term_instance_->size()?
              term_instance_->term_text_at(position - 1): "-BOS-",
          model_->yname(left_tag),
          model_->yname(tag),
          emission_cost,
          transition_cost,
          cost);
    }
    Node *node = node_pool_->Alloc();
    node->set_value(tag, min_cost, min_node);
    beam->Add(node); 
  }
}

void HMMPartOfSpeechTagger::Train(
    const char *training_corpus,
    const char *model_filename,
    Status *status) {
  ReadableFile *fd = ReadableFile::New(training_corpus, status);
  TermInstance *term_instance = new TermInstance();
  PartOfSpeechTagInstance *tag_instance = new PartOfSpeechTagInstance();
  ReimuTrie *yindex = new ReimuTrie();
  std::vector<std::string> yname;

  // First pass, gets all labels (tags)
  yindex->Put("-BOS-", HMMModel::kBeginOfSenetnceId);
  // `HMMModel::kBeginOfSenetnceId` == 0
  yname.push_back("-BOS-");
  while (status->ok() && !fd->Eof()) {
    ReadInstance(fd, term_instance, tag_instance, status);
    if (!status->ok()) break;

    for (int idx = 0; idx < tag_instance->size(); ++idx) {
      const char *tag = tag_instance->part_of_speech_tag_at(idx);
      if (yindex->Get(tag, -1) == -1) {
        // If `tag` not in `yindex`
        yindex->Put(tag, yname.size());
        yname.push_back(tag);
      }
    }
  }
  delete fd;
  fd = NULL;

  std::vector<int> y_bigram_count(yname.size() * yname.size());
  std::vector<int> y_count(yname.size());
  std::vector<int> emission;
  ReimuTrie *xindex = new ReimuTrie();
  std::vector<std::string> xname;

  // Second pass
  if (status->ok()) fd = ReadableFile::New(training_corpus, status);
  while (status->ok() && !fd->Eof()) {
    ReadInstance(fd, term_instance, tag_instance, status);
    if (!status->ok()) break;
    
    // yid(0) is `-BOS-`
    int left_yid = HMMModel::kBeginOfSenetnceId;
    int yid;
    for (int idx = 0; idx < tag_instance->size(); ++idx) {
      const char *tag = tag_instance->part_of_speech_tag_at(idx);
      const char *word = term_instance->term_text_at(idx);

      int xid = xindex->Get(word, -1);
      if (xid == -1) {
        // If `word` not in `xindex`
        xindex->Put(word, xname.size());
        xid = xname.size();
        xname.push_back(word);
        emission.resize(yname.size() * xname.size());
      }

      yid = yindex->Get(tag, -1);
      ASSERT(yid >= 0, "Invalid tag name");
      ++emission[xid * yname.size() + yid];
      ++y_count[yid];
      ++y_bigram_count[left_yid * yname.size() + yid];
      left_yid = yid;
    }
    // Increase bigram (last_word, `-BOS_`)
    ++y_count[0];
    ++y_bigram_count[yid * yname.size() + 0];
  }
  delete fd;
  fd = NULL;

  // OK, calculate and output the data
  HMMModel *hmm_model = NULL;
  if (status->ok()) {
    hmm_model = new HMMModel(yname);

    // Puts the emission data
    for (int xid = 0; xid < xname.size(); ++xid) {
      int total_count = 0;
      int size = 0;
      for (int yid = 0; yid < yname.size(); ++yid) {
        int count = emission[xid * yname.size() + yid];
        if (count != 0) ++size;
        total_count += count;
      }
      HMMModel::EmissionArray emission_array(size, total_count);
      int emission_idx = 0;
      for (int yid = 0; yid < yname.size(); ++yid) {
        int count = emission[xid * yname.size() + yid];
        if (count != 0) {
          emission_array.set_yid_at(emission_idx, yid);
          emission_array.set_cost_at(
              emission_idx,
              -log(static_cast<float>(count) / total_count));
        }
        if (count != 0) ++emission_idx;
      }
      ASSERT(emission_idx == size, "Invalid size");
      hmm_model->AddEmission(xname[xid].c_str(), emission_array);
    }

    // Puts the transition data
    for (int left_yid = 0; left_yid < yname.size(); ++left_yid) {
      for (int yid = 0; yid < yname.size(); ++yid) {
        int joint_count = 1 + y_bigram_count[left_yid * yname.size() + yid];
        int left_count = yname.size() + y_count[left_yid];
        double p_transition = static_cast<float>(joint_count) / left_count;
        hmm_model->set_cost(left_yid, yid, -log(p_transition));
      }
    }
  }

  // Save model
  if (status->ok()) hmm_model->Save(model_filename, status);

  delete term_instance;
  delete tag_instance;
  delete yindex;
  delete xindex;
  delete hmm_model;

}

}  // namespace milkcat
