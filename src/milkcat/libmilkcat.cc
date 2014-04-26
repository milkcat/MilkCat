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
// libyaca.cc
// libmilkcat.cc --- Created at 2013-09-03
//

#include "include/milkcat.h"
#include "milkcat/libmilkcat.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>
#include "common/model_factory.h"
#include "milkcat/bigram_segmenter.h"
#include "milkcat/crf_part_of_speech_tagger.h"
#include "milkcat/crf_tagger.h"
#include "milkcat/hmm_part_of_speech_tagger.h"
#include "milkcat/mixed_segmenter.h"
#include "milkcat/out_of_vocabulary_word_recognition.h"
#include "milkcat/part_of_speech_tag_instance.h"
#include "milkcat/tokenizer.h"
#include "milkcat/term_instance.h"
#include "milkcat/token_instance.h"
#include "utils/utils.h"

namespace milkcat {

Tokenization *TokenizerFactory(int analyzer_type) {
  int tokenizer_type = analyzer_type & kTokenizerMask;

  switch (tokenizer_type) {
    case TOKENIZER_NORMAL:
      return new Tokenization();

    default:
      return NULL;
  }
}

Segmenter *SegmenterFactory(ModelFactory *factory,
                            int analyzer_type,
                            Status *status) {
  int segmenter_type = analyzer_type & kSegmenterMask;

  switch (segmenter_type) {
    case SEGMENTER_BIGRAM:
      return BigramSegmenter::New(factory, true, status);

    case SEGMENTER_UNIGRAM:
      return BigramSegmenter::New(factory, false, status);

    case SEGMENTER_CRF:
      return CRFSegmenter::New(factory, status);

    case SEGMENTER_MIXED:
      return MixedSegmenter::New(factory, status);

    default:
      *status = Status::NotImplemented("Invalid segmenter type");
      return NULL;
  }
}

PartOfSpeechTagger *PartOfSpeechTaggerFactory(ModelFactory *factory,
                                              int analyzer_type,
                                              Status *status) {
  const CRFModel *crf_pos_model;
  int tagger_type = analyzer_type & kPartOfSpeechTaggerMask;

  LOG("Tagger type: %x\n", tagger_type);

  switch (tagger_type) {
    case POSTAGGER_CRF:
      if (status->ok()) crf_pos_model = factory->CRFPosModel(status);

      if (status->ok()) {
        return new CRFPartOfSpeechTagger(crf_pos_model);
      } else {
        return NULL;
      }

    case POSTAGGER_HMM:
      if (status->ok()) {
        return HMMPartOfSpeechTagger::New(factory, false, status);
      } else {
        return NULL;
      }

    case POSTAGGER_MIXED:
      if (status->ok()) {
        return HMMPartOfSpeechTagger::New(factory, true, status);
      } else {
        return NULL;
      }

    case 0:
      return NULL;

    default:
      *status = Status::NotImplemented("Invalid Part-of-speech tagger type");
      return NULL;
  }
}


// The global state saves the current of model and analyzer.
// If any errors occured, global_status != Status::OK()
Status global_status;

// ---------- Cursor ----------

Cursor::Cursor():
    analyzer_(NULL),
    tokenizer_(TokenizerFactory(TOKENIZER_NORMAL)),
    token_instance_(new TokenInstance()),
    term_instance_(new TermInstance()),
    part_of_speech_tag_instance_(new PartOfSpeechTagInstance()),
    sentence_length_(0),
    current_position_(0),
    end_(0) {
}

Cursor::~Cursor() {
  delete tokenizer_;
  tokenizer_ = NULL;

  delete token_instance_;
  token_instance_ = NULL;

  delete term_instance_;
  term_instance_ = NULL;

  delete part_of_speech_tag_instance_;
  part_of_speech_tag_instance_ = NULL;

  analyzer_ = NULL;
}

void Cursor::Scan(const char *text) {
  tokenizer_->Scan(text);
  sentence_length_ = 0;
  current_position_ = 0;
  end_ = false;
}

void Cursor::MoveToNext() {
  current_position_++;
  if (current_position_ > sentence_length_ - 1) {
    // If reached the end of current sentence

    if (tokenizer_->GetSentence(token_instance_) == false) {
      end_ = true;
    } else {
      analyzer_->segmenter->Segment(term_instance_, token_instance_);

      // If the analyzer have part of speech tagger, tag the term_instance
      if (analyzer_->part_of_speech_tagger) {
        analyzer_->part_of_speech_tagger->Tag(part_of_speech_tag_instance_,
                                              term_instance_);
      }
      sentence_length_ = term_instance_->size();
      current_position_ = 0;
    }
  }
}

}  // namespace milkcat

// ---------- Fucntions in milkcat.h ----------


milkcat_model_t *milkcat_model_new(const char *model_path) {
  if (model_path == NULL) model_path = MODEL_PATH;

  milkcat_model_t *model = new milkcat_model_t;
  model->model_factory = new milkcat::ModelFactory(model_path);

  return model;
}

milkcat_t *milkcat_new(milkcat_model_t *model, int analyzer_type) {
  milkcat::global_status = milkcat::Status::OK();

  milkcat_t *analyzer = new milkcat_t;
  memset(analyzer, 0, sizeof(milkcat_t));

  analyzer->model = model;
  analyzer->cursor = new milkcat::Cursor();

  if (milkcat::global_status.ok())
    analyzer->segmenter = milkcat::SegmenterFactory(
        analyzer->model->model_factory,
        analyzer_type,
        &milkcat::global_status);
  if (milkcat::global_status.ok())
    analyzer->part_of_speech_tagger = milkcat::PartOfSpeechTaggerFactory(
        analyzer->model->model_factory,
        analyzer_type,
        &milkcat::global_status);

  if (!milkcat::global_status.ok()) {
    milkcat_destroy(analyzer);
    return NULL;
  } else {
    return analyzer;
  }
}

void milkcat_model_destroy(milkcat_model_t *model) {
  if (model == NULL) return;

  delete model->model_factory;
  model->model_factory = NULL;

  delete model;
}

void milkcat_destroy(milkcat_t *analyzer) {
  if (analyzer == NULL) return;

  analyzer->model = NULL;

  delete analyzer->segmenter;
  analyzer->segmenter = NULL;

  delete analyzer->part_of_speech_tagger;
  analyzer->part_of_speech_tagger = NULL;

  delete analyzer->cursor;
  analyzer->cursor = NULL;

  delete analyzer;
}

void milkcat_model_set_userdict(milkcat_model_t *model, const char *path) {
  model->model_factory->SetUserDictionary(path);
}

milkcat_item_t *milkcat_analyze(milkcat_t *analyzer, const char *text) {
  // Check parameters
  if (analyzer == NULL) return NULL;

  milkcat::Cursor *cursor = analyzer->cursor;

  if (text) {
    // text != NULL, analyze new text
    cursor->set_analyzer(analyzer);
    cursor->Scan(text);    
  }

  cursor->MoveToNext();
  // If reached the end of text, collect back the cursor then return NULL
  if (cursor->end()) return NULL;

  analyzer->item.word = cursor->word();
  analyzer->item.part_of_speech_tag = cursor->part_of_speech_tag();
  analyzer->item.word_type = cursor->word_type();  

  return &(analyzer->item);
}

const char *milkcat_last_error() {
  return milkcat::global_status.what();
}

