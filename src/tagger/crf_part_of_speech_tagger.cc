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
// crf_pos_tagger.cc --- Created at 2013-10-09
// crf_part_of_speech_tagger.cc --- Created at 2013-11-24
//

#include "tagger/crf_part_of_speech_tagger.h"

#include <string.h>
#include "common/milkcat_config.h"
#include "ml/hmm_model.h"
#include "ml/sequence_feature_set.h"
#include "utils/utils.h"

namespace milkcat {


CRFPartOfSpeechTagger *CRFPartOfSpeechTagger::New(
    const CRFModel *model,
    const HMMModel *hmm_model,
    Status *status) {
  char error_message[1024];
  CRFPartOfSpeechTagger *self = new CRFPartOfSpeechTagger();
  
  self->sequence_feature_set_ = new SequenceFeatureSet();
  self->crf_tagger_ = new CRFTagger(model);
  self->hmm_model_ = hmm_model;

  self->PU_ = model->yid("PU");
  if (self->PU_ < 0) {
    *status = Status::Corruption("Unable to find tag 'PU' in CRF model");
  }

  // If `hmm_model == NULL` don't use the hmm model
  if (hmm_model == NULL) return self;

  self->hmm_crf_ymap_ = new int[hmm_model->ysize()];
  for (int hmm_tag = 0;
       hmm_tag < hmm_model->ysize() && status->ok();
       ++hmm_tag) {
    const char *hmm_yname = hmm_model->yname(hmm_tag); 
    // Ignore `-BOS-` tag
    if (strcmp(hmm_yname, "-BOS-") == 0) continue;

    int crf_yid = model->yid(hmm_yname);
    if (crf_yid < 0) {
      sprintf(error_message, "Unable to find tag '%s' in CRF model", hmm_yname);
      *status = Status::Corruption(error_message);
    } else {
      self->hmm_crf_ymap_[hmm_tag] = crf_yid;
    }
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

CRFPartOfSpeechTagger::CRFPartOfSpeechTagger(): crf_tagger_(NULL),
                                                sequence_feature_set_(NULL),
                                                hmm_model_(NULL),
                                                hmm_crf_ymap_(NULL) {
}

CRFPartOfSpeechTagger::~CRFPartOfSpeechTagger() {
  delete sequence_feature_set_;
  sequence_feature_set_ = NULL;

  delete crf_tagger_;
  crf_tagger_ = NULL;

  delete hmm_crf_ymap_;
  hmm_crf_ymap_ = NULL;
}

void CRFPartOfSpeechTagger::TagRange(
    PartOfSpeechTagInstance *tag_instance,
    TermInstance *term_instance,
    int begin,
    int end) {
  char buff[kFeatureLengthMax];

  // Prepares the `sequence_feature_set_` for tagging
  sequence_feature_set_->set_size(term_instance->size());
  for (int idx = 0; idx < term_instance->size(); ++idx) {
    int type = term_instance->term_type_at(idx);
    const char *word = term_instance->term_text_at(idx);
    int length = strlen(word);  

    FeatureSet *feature_set = sequence_feature_set_->at_index(idx);
    feature_set->Clear();

    switch (type) {
      case Parser::kChineseWord:
        // term itself
        feature_set->Add(word);
        strlcpy(buff, word, 4);
        feature_set->Add(buff);
        feature_set->Add(word + length - 3);
        sprintf(buff, "%d", length / 3);
        feature_set->Add(buff);
        break;

      case Parser::kEnglishWord:
      case Parser::kSymbol:
        feature_set->Add("A");
        feature_set->Add("A");
        feature_set->Add("A");
        feature_set->Add("1");
        break;

      case Parser::kNumber:
        feature_set->Add("1");
        feature_set->Add("1");
        feature_set->Add("1");
        feature_set->Add("1");
        break;

      default:
        feature_set->Add(".");
        feature_set->Add(".");
        feature_set->Add(".");
        feature_set->Add("1");
        break;
    }
  }

  // Use the HMM emissions
  if (hmm_model_ != NULL) {
    CRFTagger::Lattice *lattice = crf_tagger_->lattice();
    for (int idx = 0; idx < term_instance->size(); ++idx) {
      const char *word = term_instance->term_text_at(idx);
      const HMMModel::EmissionArray *emission = hmm_model_->Emission(word);
      lattice->Clear(idx);
      int term_type = term_instance->term_type_at(idx);

      // If hmm model has emission for current word, put the emission_array
      // to lattice of crf_tagger_  
      if (emission != NULL && emission->total_count() >= kEmissionThreshold) {
        for (int emission_idx = 0;
             emission_idx < emission->size();
             ++emission_idx) {
          int crf_tag = hmm_crf_ymap_[emission->yid_at(emission_idx)];
          lattice->Add(idx, crf_tag);
        }
      } else if (term_type == Parser::kPunction) {
        lattice->Add(idx, PU_);
      } else if (term_type == Parser::kOther) {
        lattice->Add(idx, PU_);
      } else {
        lattice->AllowAll(idx);
      }
    }
  }

  crf_tagger_->TagRange(sequence_feature_set_, begin, end);
  for (int i = 0; i < end - begin; ++i) {
    tag_instance->set_value_at(i, crf_tagger_->yname(crf_tagger_->y(i)));
  }
  tag_instance->set_size(end - begin);
}

}  // namespace milkcat
