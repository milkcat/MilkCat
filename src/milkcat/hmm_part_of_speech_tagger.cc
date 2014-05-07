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

#include "milkcat/hmm_part_of_speech_tagger.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <string>
#include "common/model_factory.h"
#include "milkcat/beam.h"
#include "milkcat/hmm_model.h"
#include "milkcat/libmilkcat.h"
#include "milkcat/part_of_speech_tag_instance.h"
#include "milkcat/term_instance.h"
#include "utils/utils.h"

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

CRFEmitGetter::CRFEmitGetter(): pool_top_(0),
                                feature_extractor_(NULL),
                                crf_part_of_speech_tagger_(NULL),
                                crf_tagger_(NULL),
                                probabilities_(NULL),
                                crf_to_hmm_tag_(NULL),
                                crf_tag_num_(0) {
}

CRFEmitGetter::~CRFEmitGetter() {
  delete feature_extractor_;
  feature_extractor_ = NULL;

  delete crf_part_of_speech_tagger_;
  crf_part_of_speech_tagger_ = NULL;

  delete[] probabilities_;
  probabilities_ = NULL;

  delete[] crf_to_hmm_tag_;
  crf_to_hmm_tag_ = NULL;


  std::vector<HMMModel::Emit *>::iterator it;
  for (it = emit_pool_.begin(); it != emit_pool_.end(); ++it) delete *it;
}

CRFEmitGetter *CRFEmitGetter::New(Model::Impl *model_factory, Status *status) {
  CRFEmitGetter *self = new CRFEmitGetter();
  self->feature_extractor_ = new PartOfSpeechFeatureExtractor();

  const CRFModel *crf_model = model_factory->CRFPosModel(status);
  self->hmm_model_ = NULL;

  if (status->ok()) {
    self->crf_part_of_speech_tagger_ = new CRFPartOfSpeechTagger(crf_model);
    self->crf_tagger_ = self->crf_part_of_speech_tagger_->crf_tagger();
    self->hmm_model_ = model_factory->HMMPosModel(status);
  }

  if (status->ok()) {
    self->crf_tag_num_ = crf_model->GetTagNumber();
    self->probabilities_ = new double[self->crf_tag_num_];
    self->crf_to_hmm_tag_ = new int[self->crf_tag_num_];

    for (int i = 0; i < self->crf_tag_num_; ++i) {
      self->crf_to_hmm_tag_[i] = self->hmm_model_->tag_id(
          crf_model->GetTagText(i));
    }
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

HMMModel::Emit *CRFEmitGetter::GetEmits(TermInstance *term_instance, 
                                        int position) {
  feature_extractor_->set_term_instance(term_instance);
  crf_tagger_->ProbabilityAtPosition(feature_extractor_,
                                     position,
                                     probabilities_);
  
  double max_probability = *std::max_element(probabilities_,
                                             probabilities_ + crf_tag_num_);
  HMMModel::Emit *emit = NULL, *p;
  for (int crf_tag = 0; crf_tag < crf_tag_num_; ++crf_tag) {
    if (probabilities_[crf_tag] > 0.2 * max_probability) {
      int hmm_tag = crf_to_hmm_tag_[crf_tag];
      if (hmm_tag < 0) continue;

      LOG("Add emit %s cost(T|W)=%.5lf cost(T)=%.5lf\n",
          hmm_model_->tag_str(hmm_tag),
          -log(probabilities_[crf_tag]),
          hmm_model_->tag_cost(hmm_tag));

      p = AllocEmit();
      p->tag = hmm_tag;
      p->cost = -log(probabilities_[crf_tag]) - hmm_model_->tag_cost(hmm_tag);
      p->next = emit;
      emit = p;
    }
  }

  return emit;
}

HMMPartOfSpeechTagger::HMMPartOfSpeechTagger(): node_pool_(NULL),
                                                model_(NULL),
                                                index_(NULL),
                                                crf_emit_getter_(NULL),
                                                PU_emit_(NULL),
                                                CD_emit_(NULL),
                                                NN_emit_(NULL),
                                                oov_emits_(NULL),
                                                term_instance_(NULL) {
  for (int i = 0; i < kMaxBeams; ++i) {
    beams_[i] = NULL;
  }
}

HMMPartOfSpeechTagger::~HMMPartOfSpeechTagger() {
  delete node_pool_;
  node_pool_ = NULL;

  delete PU_emit_;
  PU_emit_ = NULL;

  delete CD_emit_;
  CD_emit_ = NULL;

  delete NN_emit_;
  NN_emit_ = NULL;

  delete crf_emit_getter_;
  crf_emit_getter_ = NULL;

  for (int i = 0; i < kMaxBeams; ++i) {
    if (beams_[i] != NULL)
      delete beams_[i];
    beams_[i] = NULL;
  }

  HMMModel::Emit *p = oov_emits_, *q;
  while (p) {
    q = p->next;
    delete p;
    p = q;
  }
  oov_emits_ = NULL;
}

namespace {

// Compare two node potiners by its cost value
bool HmmNodePtrCmp(HMMPartOfSpeechTagger::Node *n1, 
                   HMMPartOfSpeechTagger::Node *n2) {
  return n1->cost < n2->cost;
}

// New an emit node for tag specified by tag_str. If tag_str not exist in model
// return a NULL and status indicates an corruption error.
HMMModel::Emit *NewEmitFromTag(const char *tag_str,
                               const HMMModel *model,
                               Status *status) {
  int tagid = model->tag_id(tag_str);
  if (tagid < 0) *status = Status::Corruption("Invalid HMM model.");

  HMMModel::Emit *emit = NULL;
  if (status->ok()) {
    emit = new HMMModel::Emit(tagid, 0.0, NULL);
  }

  return emit;
}

}  // namespace

void HMMPartOfSpeechTagger::InitEmit(HMMPartOfSpeechTagger *self,
                                     Status *status) {

  if (status->ok()) 
    self->PU_emit_ = NewEmitFromTag("PU", self->model_, status);
  if (status->ok()) 
    self->CD_emit_ = NewEmitFromTag("CD", self->model_, status);
  if (status->ok()) 
    self->NN_emit_ = NewEmitFromTag("NN", self->model_, status);

  // Create the default emits
  HMMModel::Emit *emit = NULL, *p;
  if (status->ok()) {
    p = NewEmitFromTag("NN", self->model_, status);
    p->next = emit;
    emit = p;
  }
  if (status->ok()) {
    p = NewEmitFromTag("VV", self->model_, status);
    p->next = emit;
    emit = p;
  }
  if (status->ok()) {
    p = NewEmitFromTag("VA", self->model_, status);
    p->next = emit;
    emit = p;
  }
  if (status->ok()) {
    p = NewEmitFromTag("AD", self->model_, status);
    p->next = emit;
    emit = p;   
  }
  if (status->ok()) {
    p = NewEmitFromTag("NR", self->model_, status);
    p->next = emit;
    self->oov_emits_ = p;   
  }
}

HMMPartOfSpeechTagger *HMMPartOfSpeechTagger::New(
    Model::Impl *model_factory,
    bool use_crf,
    Status *status) {
  HMMPartOfSpeechTagger *self = new HMMPartOfSpeechTagger();
  self->node_pool_ = new utils::Pool<Node>(); 

  for (int i = 0; i < kMaxBeams; ++i) {
    self->beams_[i] = new Beam<Node>(kBeamSize,
                                     self->node_pool_,
                                     i,
                                     HmmNodePtrCmp);
  }

  self->model_ = model_factory->HMMPosModel(status);

  if (status->ok()) InitEmit(self, status);
  if (status->ok() && use_crf) 
    self->crf_emit_getter_ = CRFEmitGetter::New(model_factory, status);
  if (status->ok()) {
    self->BOS_tagid_ = self->model_->tag_id("BOS");
    if (self->BOS_tagid_ < 0) 
      *status = Status::Corruption("Invalid HMM model.");
  }

  if (status->ok()) {
    self->NN_tagid_ = self->model_->tag_id("NN");
    if (self->NN_tagid_ < 0) 
      *status = Status::Corruption("Invalid HMM model.");
  }

  if (status->ok()) self->index_ = model_factory->Index(status);

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

HMMModel::Emit *HMMPartOfSpeechTagger::GetEmitAtPosition(int position) {
  int term_id;
  HMMModel::Emit *emit;

  term_id = term_instance_->term_id_at(position);
  if (term_id == TermInstance::kTermIdNone) {
    term_id = index_->Search(term_instance_->term_text_at(position));
  }

  emit = model_->emit(term_id);
  if (emit == NULL) {
    int term_type = term_instance_->term_type_at(position);
    switch (term_type) {
      case Parser::kPunction:
      case Parser::kSymbol:
      case Parser::kOther:
        emit = PU_emit_;
        break;
      case Parser::kNumber:
        emit = CD_emit_;
        break;
      case Parser::kEnglishWord:
        emit = NN_emit_;
        break;
    }
  }

  if (emit == NULL && crf_emit_getter_) {
      emit = crf_emit_getter_->GetEmits(term_instance_, position);
  }

  if (emit == NULL) emit = oov_emits_;

  return emit;
}

inline void HMMPartOfSpeechTagger::AddBOSNodeToBeam() {
  Node *bos_node = node_pool_->Alloc();
  bos_node->set_value(BOS_tagid_, 0, NULL);
  beams_[0]->Clear();
  beams_[0]->Add(bos_node);

  Node *bos_node_2 = node_pool_->Alloc();
  bos_node_2->set_value(BOS_tagid_, 0, bos_node);
  bos_node_2->tag = BOS_tagid_;
  beams_[1]->Clear();
  beams_[1]->Add(bos_node_2);
}

inline void HMMPartOfSpeechTagger::GetBestPOSTagFromBeam(
    PartOfSpeechTagInstance *part_of_speech_tag_instance) {
  int position = term_instance_->size() - 1;
  const Node *node = beams_[position + 2]->MinimalNode();

  while (node->tag != BOS_tagid_) {
    part_of_speech_tag_instance->set_value_at(position,
                                              model_->tag_str(node->tag));
    position--;
    node = node->prevoius_node;
  }

  part_of_speech_tag_instance->set_size(term_instance_->size());
}

void HMMPartOfSpeechTagger::Tag(
    PartOfSpeechTagInstance *part_of_speech_tag_instance,
    TermInstance *term_instance) {
  term_instance_ = term_instance;

  AddBOSNodeToBeam();

  // Viterbi algorithm
  for (int i = 0; i < term_instance->size(); ++i)
    BuildBeam(i);

  // Find the best result
  GetBestPOSTagFromBeam(part_of_speech_tag_instance);

  node_pool_->ReleaseAll();
}

void HMMPartOfSpeechTagger::BuildBeam(int position) {
  // Beam has two BOS node at 0 and 1
  int beam_position = position + 2;

  HMMModel::Emit *emit = GetEmitAtPosition(position);

  const Node *leftleft_node, *left_node;
  int leftleft_tag, left_tag;

  Beam<Node> *previous_beam = beams_[beam_position - 1];
  Beam<Node> *beam = beams_[beam_position];

  previous_beam->Shrink();
  beam->Clear();

  double min_cost;
  const Node *min_left_node;
  while (emit) {
    min_cost = 1e38;
    min_left_node = NULL;
    for (int i = 0; i < previous_beam->size(); ++i) {
      left_node = previous_beam->node_at(i);
      leftleft_node = left_node->prevoius_node;

      leftleft_tag = leftleft_node->tag;
      left_tag = left_node->tag;

      // If current word has emit information or current word is punction or 
      // symbol

      double trans_cost = model_->trans_cost(leftleft_tag, 
                                             left_tag, 
                                             emit->tag);
      double emit_cost = emit->cost;
      double cost = left_node->cost + trans_cost + emit_cost;
      if (cost < min_cost) {
        min_cost = cost;
        min_left_node = left_node;
      }

      LOG("%s %s %s %s "
          "total_cost = %lf cost = %lf trans_cost = %lf emit_cost = %lf\n",
          term_instance_->term_text_at(position),
          model_->tag_str(leftleft_tag), 
          model_->tag_str(left_tag),
          model_->tag_str(emit->tag),
          cost,
          trans_cost + emit_cost,
          trans_cost,
          emit_cost);
    }

    Node *node = node_pool_->Alloc();
    node->set_value(emit->tag, min_cost, min_left_node);
    beam->Add(node); 

    emit = emit->next;
  }

  if (crf_emit_getter_) crf_emit_getter_->ReleaseAllEmits();
}

}  // namespace milkcat
