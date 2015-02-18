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
#include "segmenter/crf_segmenter.h"
#include "segmenter/mixed_segmenter.h"
#include "segmenter/out_of_vocabulary_word_recognition.h"
#include "segmenter/term_instance.h"
#include "tagger/crf_part_of_speech_tagger.h"
#include "tagger/hmm_part_of_speech_tagger.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "parser/beam_yamada_parser.h"
#include "parser/dependency_parser.h"
#include "parser/tree_instance.h"
#include "parser/yamada_parser.h"
#include "tokenizer/tokenizer.h"
#include "tokenizer/token_instance.h"
#include "util/util.h"

namespace milkcat {

// This enum represents the type or the algorithm of Parser. It could be
// kDefault which indicates using the default algorithm for segmentation and
// part-of-speech tagging. Meanwhile, it could also be
//   kTextTokenizer | kBigramSegmenter | kHmmTagger
// which indicates using bigram segmenter for segmentation, using HMM model
// for part-of-speech tagging.
enum ParserType {
  // Tokenizer type
  kTextTokenizer = 0,

  // Segmenter type
  kMixedSegmenter = 0x00000000,
  kCrfSegmenter = 0x00000010,
  kUnigramSegmenter = 0x00000020,
  kBigramSegmenter = 0x00000030,

  // Part-of-speech tagger type
  kMixedTagger = 0x00000000,
  kHmmTagger = 0x00001000,
  kCrfTagger = 0x00002000,
  kNoTagger = 0x000ff000,

  // Depengency parser type
  kYamadaParser = 0x00100000,
  kBeamYamadaParser = 0x00200000,
  kNoParser = 0x00000000
};

Tokenization *TokenizerFactory(int analyzer_type) {
  int tokenizer_type = analyzer_type & kTokenizerMask;

  switch (tokenizer_type) {
    case kTextTokenizer:
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
    case kBigramSegmenter:
      return BigramSegmenter::New(factory, true, status);

    case kUnigramSegmenter:
      return BigramSegmenter::New(factory, false, status);

    case kCrfSegmenter:
      return CRFSegmenter::New(factory, status);

    case kMixedSegmenter:
      return MixedSegmenter::New(factory, status);

    default:
      *status = Status::NotImplemented("Invalid segmenter type");
      return NULL;
  }
}

PartOfSpeechTagger *PartOfSpeechTaggerFactory(Model::Impl *factory,
                                              int analyzer_type,
                                              Status *status) {
  const CRFModel *crf_pos_model = NULL;
  const HMMModel *hmm_pos_model = NULL;
  int tagger_type = analyzer_type & kPartOfSpeechTaggerMask;

  switch (tagger_type) {
    case kCrfTagger:
      if (status->ok()) crf_pos_model = factory->CRFPosModel(status);

      if (status->ok()) {
        return CRFPartOfSpeechTagger::New(crf_pos_model, NULL, status);
      } else {
        return NULL;
      }

    case kHmmTagger:
      if (status->ok()) {
        return HMMPartOfSpeechTagger::New(factory, status);
      } else {
        return NULL;
      }

    case kMixedTagger:
      if (status->ok()) crf_pos_model = factory->CRFPosModel(status);
      if (status->ok()) hmm_pos_model = factory->HMMPosModel(status);
      if (status->ok()) {
        return CRFPartOfSpeechTagger::New(crf_pos_model, hmm_pos_model, status);
      } else {
        return NULL;
      }

    case kNoTagger:
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
    case kYamadaParser:
      if (status->ok()) {
        return YamadaParser::New(factory, status);
      } else {
        return NULL;
      }

    case kBeamYamadaParser:
      if (status->ok()) {
        return BeamYamadaParser::New(factory, status);
      } else {
        return NULL;
      }

    case kNoParser:
      return NULL;

    default:
      *status = Status::NotImplemented("Invalid parser type");
      return NULL;    
  }
}

char gLastErrorMessage[kLastErrorStringMax] = "";

// ----------------------------- Parser::Iterator -----------------------------

Parser::Iterator::Impl::Impl():
    parser_(NULL),
    tokenizer_(TokenizerFactory(kTextTokenizer)),
    token_instance_(new TokenInstance()),
    term_instance_(new TermInstance()),
    tree_instance_(new TreeInstance()),
    part_of_speech_tag_instance_(new PartOfSpeechTagInstance()),
    sentence_length_(0),
    current_position_(0),
    end_(true),
    is_begin_of_sentence_(true) {
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

  delete tree_instance_;
  tree_instance_ = NULL;

  parser_ = NULL;
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
      parser_->segmenter()->Segment(term_instance_, token_instance_);

      // If the parser have part-of-speech tagger, tag the term_instance
      if (parser_->part_of_speech_tagger()) {
        parser_->part_of_speech_tagger()->Tag(part_of_speech_tag_instance_,
                                              term_instance_);
      }

      // Dependency Parsing
      if (parser_->dependency_parser()) {
        parser_->dependency_parser()->Parse(tree_instance_,
                                            term_instance_,
                                            part_of_speech_tag_instance_);
      }
      sentence_length_ = term_instance_->size();
      current_position_ = 0;
      is_begin_of_sentence_ = true;
    }
  } else {
    is_begin_of_sentence_ = false;
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
int Parser::Iterator::head() const {
  return impl_->head();
}
const char *Parser::Iterator::dependency_label() const {
  return impl_->dependency_label();
}
bool Parser::Iterator::is_begin_of_sentence() const {
  return impl_->is_begin_of_sentence();
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
}

Parser::Impl *Parser::Impl::New(const Options &options, Model *model) {
  Status status = Status::OK();
  Impl *self = new Parser::Impl();
  int type = options.TypeValue();
  Model::Impl *model_impl = model? model->impl(): NULL;

  if (model_impl == NULL) {
    self->model_impl_ = new Model::Impl(MODEL_DIR);
    self->own_model_ = true;
  } else {
    self->model_impl_ = model_impl;
    self->own_model_ = false;
  }

  if (status.ok())
    self->segmenter_ = SegmenterFactory(
        self->model_impl_,
        type,
        &status);

  if (status.ok())
    self->part_of_speech_tagger_ = PartOfSpeechTaggerFactory(
        self->model_impl_,
        type,
        &status);

  if (status.ok())
    self->dependency_parser_ = DependencyParserFactory(
        self->model_impl_,
        type,
        &status);

  if (!status.ok()) {
    strlcpy(gLastErrorMessage, status.what(), sizeof(gLastErrorMessage));
    delete self;
    return NULL;
  } else {
    return self;
  }
}

void Parser::Impl::Predict(Parser::Iterator *iterator, const char *text) {
  iterator->impl()->set_parser(this);
  iterator->impl()->Scan(text);
  iterator->Next();
}

Parser::Parser(): impl_(NULL) {
}

Parser::~Parser() {
  delete impl_;
  impl_ = NULL;
}

Parser *Parser::New() {
  return New(Options(), NULL);
}

Parser *Parser::New(const Options &options) {
  return New(options, NULL);
}

Parser *Parser::New(const Options &options, Model *model) {
  Parser *self = new Parser();
  self->impl_ = Impl::New(options, model);

  if (self->impl_) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

void Parser::Predict(Parser::Iterator *iterator, const char *text) {
  return impl_->Predict(iterator, text);
}

Parser::Options::Options(): segmenter_type_(kMixedSegmenter),
                            tagger_type_(kMixedTagger),
                            parser_type_(kNoParser) {
}

void Parser::Options::UseMixedSegmenter() {
  segmenter_type_ = kMixedSegmenter;
}
void Parser::Options::UseCrfSegmenter() {
  segmenter_type_ = kCrfSegmenter;
}
void Parser::Options::UseUnigramSegmenter() {
  segmenter_type_ = kUnigramSegmenter;
}
void Parser::Options::UseBigramSegmenter() {
  segmenter_type_ = kBigramSegmenter;
}
void Parser::Options::UseMixedPOSTagger() {
  tagger_type_ = kMixedTagger;
}
void Parser::Options::UseHmmPOSTagger() {
  tagger_type_ = kHmmTagger;
}
void Parser::Options::UseCrfPOSTagger() {
  tagger_type_ = kCrfTagger;
}
void Parser::Options::NoPOSTagger() {
  tagger_type_ = kNoTagger;
}
void Parser::Options::UseBeamYamadaParser() {
  if (tagger_type_ == kNoTagger) tagger_type_ = kMixedTagger;
  parser_type_ = kBeamYamadaParser;
}
void Parser::Options::UseYamadaParser() {
  if (tagger_type_ == kNoTagger) tagger_type_ = kMixedTagger;
  parser_type_ = kYamadaParser;
}
void Parser::Options::NoDependencyParser() {
  parser_type_ = kNoParser;
}
int Parser::Options::TypeValue() const {
  return segmenter_type_ | tagger_type_ | parser_type_;
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
  self->impl_ = new Model::Impl(model_dir? model_dir: MODEL_DIR);

  return self;
}

bool Model::SetUserDictionary(const char *userdict_path) {
  return impl_->SetUserDictionary(userdict_path);
}

const char *LastError() {
  return gLastErrorMessage;
}

}  // namespace milkcat
