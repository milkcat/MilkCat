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
#include "libmilkcat.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>
#include "common/model_impl.h"
#include "ml/crf_tagger.h"
#include "segmenter/bigram_segmenter.h"
#include "segmenter/mixed_segmenter.h"
#include "segmenter/out_of_vocabulary_word_recognition.h"
#include "segmenter/term_instance.h"
#include "tagger/crf_part_of_speech_tagger.h"
#include "tagger/hmm_part_of_speech_tagger.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "parser/dependency_instance.h"
#include "parser/dependency_parser.h"
#include "tokenizer/tokenizer.h"
#include "tokenizer/token_instance.h"
#include "utils/log.h"
#include "utils/utils.h"

namespace milkcat {

Tokenization *TokenizerFactory(int analyzer_type) {
  int tokenizer_type = analyzer_type & kTokenizerMask;

  switch (tokenizer_type) {
    case Parser::kTextTokenizer:
      return new Tokenization();

    default:
      return NULL;
  }
}

Segmenter *SegmenterFactory(Model::Impl *factory,
                            int analyzer_type,
                            Status *status) {
  int segmenter_type = analyzer_type & kSegmenterMask;

  switch (segmenter_type) {
    case Parser::kBigramSegmenter:
      return BigramSegmenter::New(factory, true, status);

    case Parser::kUnigramSegmenter:
      return BigramSegmenter::New(factory, false, status);

    case Parser::kCrfSegmenter:
      return CRFSegmenter::New(factory, status);

    case Parser::kMixedSegmenter:
      return MixedSegmenter::New(factory, status);

    default:
      *status = Status::NotImplemented("Invalid segmenter type");
      return NULL;
  }
}

PartOfSpeechTagger *PartOfSpeechTaggerFactory(Model::Impl *factory,
                                              int analyzer_type,
                                              Status *status) {
  const CRFModel *crf_pos_model;
  int tagger_type = analyzer_type & kPartOfSpeechTaggerMask;

  LOG("Tagger type: " << tagger_type);

  switch (tagger_type) {
    case Parser::kCrfTagger:
      if (status->ok()) crf_pos_model = factory->CRFPosModel(status);

      if (status->ok()) {
        return new CRFPartOfSpeechTagger(crf_pos_model);
      } else {
        return NULL;
      }

    case Parser::kHmmTagger:
      if (status->ok()) {
        return HMMPartOfSpeechTagger::New(factory, false, status);
      } else {
        return NULL;
      }

    case Parser::kMixedTagger:
      if (status->ok()) {
        return HMMPartOfSpeechTagger::New(factory, true, status);
      } else {
        return NULL;
      }

    case Parser::kNoTagger:
      return NULL;

    default:
      *status = Status::NotImplemented("Invalid Part-of-speech tagger type");
      return NULL;
  }
}

DependencyParser *DependencyParserFactory(Model::Impl *factory,
                                          int parser_type,
                                          Status *status) {
  parser_type = kParserMask & parser_type;
  switch (parser_type) {
    case Parser::kMaxentParser:
      if (status->ok()) {
        return DependencyParser::New(factory, status);
      } else {
        return NULL;
      }

    case Parser::kNoParser:
      return NULL;

    default:
      *status = Status::NotImplemented("Invalid parser type");
      return NULL;    
  }
}

// The global state saves the current of model and analyzer.
// If any errors occured, global_status != Status::OK()
Status global_status;

// ----------------------------- Parser::Iterator -----------------------------

Parser::Iterator::Impl::Impl():
    analyzer_(NULL),
    tokenizer_(TokenizerFactory(Parser::kTextTokenizer)),
    token_instance_(new TokenInstance()),
    term_instance_(new TermInstance()),
    dependency_instance_(new DependencyInstance()),
    part_of_speech_tag_instance_(new PartOfSpeechTagInstance()),
    sentence_length_(0),
    current_position_(0),
    end_(false) {
}

Parser::Iterator::Impl::~Impl() {
  delete tokenizer_;
  tokenizer_ = NULL;

  delete token_instance_;
  token_instance_ = NULL;

  delete term_instance_;
  term_instance_ = NULL;

  delete part_of_speech_tag_instance_;
  part_of_speech_tag_instance_ = NULL;

  delete dependency_instance_;
  dependency_instance_ = NULL;

  analyzer_ = NULL;
}

void Parser::Iterator::Impl::Scan(const char *text) {
  tokenizer_->Scan(text);
  sentence_length_ = 0;
  current_position_ = 0;
  end_ = false;
}

void Parser::Iterator::Impl::Next() {
  if (End()) return ;
  
  current_position_++;
  if (current_position_ > sentence_length_ - 1) {
    // If reached the end of current sentence

    if (tokenizer_->GetSentence(token_instance_) == false) {
      end_ = true;
    } else {
      analyzer_->segmenter()->Segment(term_instance_, token_instance_);

      // If the analyzer have part of speech tagger, tag the term_instance
      if (analyzer_->part_of_speech_tagger()) {
        analyzer_->part_of_speech_tagger()->Tag(part_of_speech_tag_instance_,
                                                term_instance_);
      }

      // Dependency Parsing
      if (analyzer_->dependency_parser()) {
        analyzer_->dependency_parser()->Parse(dependency_instance_,
                                              part_of_speech_tag_instance_,
                                              term_instance_);
      }
      sentence_length_ = term_instance_->size();
      current_position_ = 0;
    }
  }
}


Parser::Iterator::Iterator() {
  impl_ = new Parser::Iterator::Impl();
}

Parser::Iterator::~Iterator() {
  delete impl_;
  impl_ = NULL;
}

bool Parser::Iterator::End() {
  return impl_->End();
}

void Parser::Iterator::Next() {
  impl_->Next();
}

const char *Parser::Iterator::word() const {
  return impl_->word();
}
const char *Parser::Iterator::part_of_speech_tag() const {
  return impl_->part_of_speech_tag();
}
int Parser::Iterator::type() const {
  return impl_->type();
}
int Parser::Iterator::head_node() const {
  return impl_->head_node();
}
const char *Parser::Iterator::dependency_type() const {
  return impl_->dependency_type();
}


// ----------------------------- Parser --------------------------------------

Parser::Impl::Impl(): segmenter_(NULL),
                      part_of_speech_tagger_(NULL),
                      dependency_parser_(NULL),
                      model_impl_(NULL),
                      own_model_(false),
                      iterator_alloced_(0) {
}

Parser::Impl::~Impl() {
  delete segmenter_;
  segmenter_ = NULL;

  delete part_of_speech_tagger_;
  part_of_speech_tagger_ = NULL;

  delete dependency_parser_;
  dependency_parser_ = NULL;

  if (own_model_) delete model_impl_;
  model_impl_ = NULL;

  for (std::vector<Parser::Iterator *>::iterator
       it = iterator_pool_.begin(); it != iterator_pool_.end(); ++it) {
    delete *it;
  }
}

Parser::Impl *Parser::Impl::New(Model::Impl *model_impl, int type) {
  global_status = Status::OK();
  Impl *self = new Parser::Impl();

  if (model_impl == NULL) {
    self->model_impl_ = new Model::Impl(MODEL_PATH);
    self->own_model_ = true;
  } else {
    self->model_impl_ = model_impl;
    self->own_model_ = false;
  }

  if (global_status.ok())
    self->segmenter_ = SegmenterFactory(
        self->model_impl_,
        type,
        &global_status);

  if (global_status.ok())
    self->part_of_speech_tagger_ = PartOfSpeechTaggerFactory(
        self->model_impl_,
        type,
        &global_status);

  if (global_status.ok())
    self->dependency_parser_ = DependencyParserFactory(
        self->model_impl_,
        type,
        &global_status);

  if (!global_status.ok()) {
    delete self;
    return NULL;
  } else {
    return self;
  }
}

Parser::Iterator *Parser::Impl::Parse(const char *text) {
  Parser::Iterator *cursor;
  if (iterator_pool_.size() == 0) {
    ASSERT(iterator_alloced_ < 1024,
           "Too many Parser::Iterator allocated without Parser::Release.");
    cursor = new Parser::Iterator();
    iterator_alloced_++;
  } else {
    cursor = iterator_pool_.back();
    iterator_pool_.pop_back();
  }

  cursor->impl()->set_analyzer(this);
  cursor->impl()->Scan(text);
  cursor->Next();

  return cursor;
}

Parser::Parser(): impl_(NULL) {
}

Parser::~Parser() {
  delete impl_;
  impl_ = NULL;
}

Parser *Parser::New(Model *model, int type) {
  Parser *self = new Parser();
  self->impl_ = Impl::New(model? model->impl(): NULL, type);

  if (self->impl_) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

void Parser::Release(Parser::Iterator *it) {
  return impl_->Release(it);
}

Parser::Iterator *Parser::Parse(const char *text) {
  return impl_->Parse(text);
}

// ------------------------------- Model -------------------------------------

Model::Model(): impl_(NULL) {
}

Model::~Model() {
  delete impl_;
  impl_ = NULL;
}

Model *Model::New(const char *model_dir) {
  Model *self = new Model();
  self->impl_ = new Model::Impl(model_dir? model_dir: MODEL_PATH);

  return self;
}

bool Model::SetUserDictionary(const char *userdict_path) {
  return impl_->SetUserDictionary(userdict_path);
}

const char *LastError() {
  return global_status.what();
}

}  // namespace milkcat
