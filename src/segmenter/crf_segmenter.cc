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
// tag_segmenter.cc --- Created at 2013-02-12
// crf_segmenter.cc --- Created at 2013-08-17
//


#include <stdio.h>
#include <string.h>
#include <string>
#include "libmilkcat.h"
#include "common/model_impl.h"
#include "ml/sequence_feature_set.h"
#include "segmenter/crf_segmenter.h"
#include "segmenter/term_instance.h"
#include "tokenizer/token_instance.h"
#include "util/util.h"

namespace milkcat {

CRFSegmenter *CRFSegmenter::New(Model::Impl *model_factory, Status *status) {
  CRFSegmenter *self = new CRFSegmenter();
  const CRFModel *model = model_factory->CRFSegModel(status);
  
  if (status->ok()) {
    self->crf_tagger_ = new CRFTagger(model);
    self->sequence_feature_set_ = new SequenceFeatureSet();

    // Get the tag's value in CRF++ model
    self->S = self->crf_tagger_->yid("S");
    self->B = self->crf_tagger_->yid("B");
    self->B1 = self->crf_tagger_->yid("B1");
    self->B2 = self->crf_tagger_->yid("B2");
    self->M = self->crf_tagger_->yid("M");
    self->E = self->crf_tagger_->yid("E");

    if (self->S < 0 || self->B < 0 || self->B1 < 0 || self->B2 < 0 ||
        self->M < 0 || self->E < 0) {
      *status = Status::Corruption(
        "bad CRF++ segmenter model, unable to find S, B, B1, B2, M, E tag.");
    }    
  }

  if (status->ok()) {
    CRFTagger::TransitionTable *
    transition_table = self->crf_tagger_->transition_table();

    // Speed up decoding by only calculating the specified transitions
    transition_table->DisallowAll();
    transition_table->Allow(self->S, self->S);
    transition_table->Allow(self->S, self->B);
    transition_table->Allow(self->B, self->B1);
    transition_table->Allow(self->B, self->E);
    transition_table->Allow(self->B1, self->B2);
    transition_table->Allow(self->B1, self->E);
    transition_table->Allow(self->B2, self->M);
    transition_table->Allow(self->B2, self->E);
    transition_table->Allow(self->M, self->M);
    transition_table->Allow(self->M, self->E);
    transition_table->Allow(self->E, self->S);
    transition_table->Allow(self->E, self->B);
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

CRFSegmenter::~CRFSegmenter() {
  delete sequence_feature_set_;
  sequence_feature_set_ = NULL;

  delete crf_tagger_;
  crf_tagger_ = NULL;
}

CRFSegmenter::CRFSegmenter(): crf_tagger_(NULL),
                              sequence_feature_set_(NULL) {}

void CRFSegmenter::SegmentRange(TermInstance *term_instance,
                                TokenInstance *token_instance,
                                int begin,
                                int end) {
  std::string buffer;

  // Puts the `token_instance` into `sequence_feature_set_`
  CRFTagger::Lattice *lattice = crf_tagger_->lattice();
  sequence_feature_set_->set_size(token_instance->size());
  for (int idx = 0; idx < token_instance->size(); ++idx) {
    FeatureSet *feature_set = sequence_feature_set_->at_index(idx);

    // Only one feature for word segmenter
    feature_set->Clear();
    feature_set->Add(token_instance->token_text_at(idx));

    int token_type = token_instance->token_type_at(idx);
    if (token_type == TokenInstance::kChineseChar) {
      lattice->AllowAll(idx);
    } else {
      // For other tokens just assign S tag
      lattice->Clear(idx);
      lattice->Add(idx, S);
    }
  }

  crf_tagger_->TagRange(sequence_feature_set_, begin, end, S, S);

  int tag_id;
  int term_count = 0;
  size_t i = 0;
  int token_count = 0;
  int term_type;
  for (i = 0; i < end - begin; ++i) {
    token_count++;
    buffer.append(token_instance->token_text_at(begin + i));

    tag_id = crf_tagger_->y(i);
    if (tag_id == S || tag_id == E) {
      if (tag_id == S) {
        term_type = TokenTypeToTermType(
          token_instance->token_type_at(begin + i));
      } else {
        term_type = Parser::kChineseWord;
      }

      term_instance->set_value_at(term_count,
                                  buffer.c_str(),
                                  token_count,
                                  term_type);
      term_count++;
      token_count = 0;
      buffer.clear();
    }
  }

  if (!buffer.empty()) {
    term_instance->set_value_at(term_count,
                                buffer.c_str(),
                                token_count,
                                Parser::kChineseWord);
    term_count++;
  }

  term_instance->set_size(term_count);
}

}  // namespace milkcat
