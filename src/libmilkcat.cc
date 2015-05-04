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
#include "common/model.h"
#include "ml/crf_tagger.h"
#include "segmenter/bigram_segmenter.h"
#include "segmenter/crf_segmenter.h"
#include "segmenter/mixed_segmenter.h"
#include "segmenter/out_of_vocabulary_word_recognizer.h"
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

Tokenizer *TokenizerFactory(int analyzer_type) {
  int tokenizer_type = analyzer_type & kTokenizerMask;

  switch (tokenizer_type) {
    case kTextTokenizer:
      return new Tokenizer();

    default:
      return NULL;
  }
}

Segmenter *SegmenterFactory(Model *factory,
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

PartOfSpeechTagger *PartOfSpeechTaggerFactory(Model *factory,
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

DependencyParser *DependencyParserFactory(Model *factory,
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
    sentence_size_(0),
    current_idx_(0),
    end_(true),
    use_gbk_(false),
    segmenter_(NULL),
    postagger_(NULL),
    dependency_parser_(NULL) {
  sentence_ = new SentenceInstance();
  tokenizer_ = new Tokenizer();
  encoding_ = new Encoding();
}

Parser::Iterator::Impl::~Impl() {
  delete sentence_;
  sentence_ = NULL;

  delete tokenizer_;
  tokenizer_ = NULL;

  delete encoding_;
  encoding_ = NULL;
}

void Parser::Iterator::Impl::Reset(
    Segmenter *segmenter,
    PartOfSpeechTagger *postagger,
    DependencyParser *dependency_parser,
    bool use_gbk,
    const char *text) {
  segmenter_ = segmenter;
  postagger_ = postagger;
  dependency_parser_ = dependency_parser;

  sentence_size_ = 0;
  current_idx_ = -1;
  end_ = false;
  use_gbk_ = use_gbk;

  tokenizer_->Scan(text);
}

bool Parser::Iterator::Impl::Next() {
  if (end_) return false;

  assert(sentence_size_ > current_idx_);
  if (sentence_size_ == current_idx_ + 1) {
    // Tokenization
    TokenInstance *token_instance = sentence_->token_instance();
    bool has_next = tokenizer_->GetSentence(token_instance);
    if (has_next == false) {
      end_ = true;
      return false;
    }

    // Word breaking
    TermInstance *term_instance = sentence_->term_instance();
    segmenter_->Segment(term_instance, token_instance);

    // Part-of-speech tagging
    PartOfSpeechTagInstance *postag_instance;
    if (postagger_ != NULL) {
      postag_instance = sentence_->part_of_speech_tag_instance();
      postagger_->Tag(postag_instance, term_instance);
    }

    // Dependency parsing
    if (dependency_parser_ != NULL) {
      TreeInstance *tree_instance = sentence_->tree_instance();
      dependency_parser_->Parse(tree_instance,
                                term_instance,
                                postag_instance);
    }

    // Converts to GBK when needed
    if (use_gbk_) ConvertToGBKTermInstance(term_instance);

    // Reset cursor
    sentence_size_ = term_instance->size();
    current_idx_ = 0;
  } else {
    ++current_idx_;
  }

  return true;
}

void Parser::Iterator::Impl::ConvertToGBKTermInstance(
    TermInstance *term_instance) {
  char gbk_string[kTermLengthMax * 2];
  for (int idx = 0; idx < term_instance->size(); ++idx) {
    encoding_->UTF8ToGBK(
        term_instance->term_text_at(idx),
        gbk_string,
        sizeof(gbk_string));
    term_instance->set_value_at(
        idx,
        gbk_string,
        term_instance->token_number_at(idx),
        term_instance->term_type_at(idx),
        term_instance->term_id_at(idx));
  }
}

Parser::Iterator::Iterator() {
  impl_ = new Parser::Iterator::Impl();
}

Parser::Iterator::~Iterator() {
  delete impl_;
  impl_ = NULL;
}

bool Parser::Iterator::Next() {
  return impl_->Next();
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
                      model_(NULL),
                      use_gbk_(false),
                      utf8_buffersize_(1024) {
  utf8_buffer_ = new char[1024];
}

Parser::Impl::~Impl() {
  delete segmenter_;
  segmenter_ = NULL;

  delete part_of_speech_tagger_;
  part_of_speech_tagger_ = NULL;

  delete dependency_parser_;
  dependency_parser_ = NULL;

  delete model_;
  model_ = NULL;

  delete[] utf8_buffer_;
  utf8_buffer_ = NULL;
}

Parser::Impl *Parser::Impl::New(const Options &options) {
  Status status = Status::OK();
  Impl *self = new Parser::Impl();
  int type = options.impl()->TypeValue();

  self->use_gbk_ = options.impl()->use_gbk();

  if (strcmp(options.impl()->model_path(), "") == 0) {
    self->model_ = new Model(MODEL_DIR);
  } else {
    self->model_ = new Model(options.impl()->model_path());
  }

  if (status.ok() && strcmp(options.impl()->user_dictionary(), "") != 0) {
    self->model_->ReadUserDictionary(options.impl()->user_dictionary(),
                                     &status);
  }

  if (status.ok())
    self->segmenter_ = SegmenterFactory(
        self->model_,
        type,
        &status);

  if (status.ok())
    self->part_of_speech_tagger_ = PartOfSpeechTaggerFactory(
        self->model_,
        type,
        &status);

  if (status.ok())
    self->dependency_parser_ = DependencyParserFactory(
        self->model_,
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
  // Ensures when dependency_parser_ exists part_of_speech_tagger_ should be
  // exist
  assert(dependency_parser_ != NULL? part_of_speech_tagger_ != NULL: true);

  if (iterator == NULL) return ;
  Iterator::Impl *iterator_impl = iterator->impl();

  // Tokenization
  if (use_gbk_) {
    // When using GBK encoding
    int len = static_cast<int>(strlen(text));

    // Requires enough space for utf8 string
    int required = 2 * (len + 1);
    if (utf8_buffersize_ < required) {
      delete[] utf8_buffer_;
      utf8_buffer_ = new char[required];
      utf8_buffersize_ = required;
    }

    iterator_impl->encoding()->GBKToUTF8(text, utf8_buffer_, utf8_buffersize_);
    text = utf8_buffer_;
  }

  iterator_impl->Reset(
      segmenter_,
      part_of_speech_tagger_,
      dependency_parser_,
      use_gbk_,
      text);
}

Parser::~Parser() {
  delete impl_;
  impl_ = NULL;
}

Parser::Parser(): impl_(Impl::New(Options())) {
}
Parser::Parser(const Options &options): impl_(Impl::New(options)) {
}

void Parser::Predict(Parser::Iterator *iterator, const char *text) {
  if (impl_ == NULL) return ;
  return impl_->Predict(iterator, text);
}

Parser::Options::Options(): impl_(new Impl()) {
}
Parser::Options::~Options() {
  delete impl_;
  impl_ = NULL;
}

Parser::Options::Impl::Impl():
    segmenter_type_(kMixedSegmenter),
    tagger_type_(kMixedTagger),
    parser_type_(kNoParser),
    use_gbk_(false) {
}
void Parser::Options::UseGBK() {
  impl_->UseGBK();
}
void Parser::Options::UseUTF8() {
  impl_->UseUTF8();
}
void Parser::Options::UseMixedSegmenter() {
  impl_->UseMixedSegmenter();
}
void Parser::Options::UseCRFSegmenter() {
  impl_->UseCRFSegmenter();
}
void Parser::Options::UseUnigramSegmenter() {
  impl_->UseUnigramSegmenter();
}
void Parser::Options::UseBigramSegmenter() {
  impl_->UseBigramSegmenter();
}
void Parser::Options::UseMixedPOSTagger() {
  impl_->UseMixedPOSTagger();
}
void Parser::Options::UseHMMPOSTagger() {
  impl_->UseHMMPOSTagger();
}
void Parser::Options::UseCRFPOSTagger() {
  impl_->UseCRFPOSTagger();
}
void Parser::Options::NoPOSTagger() {
  impl_->NoPOSTagger();
}
void Parser::Options::UseBeamYamadaParser() {
  impl_->UseBeamYamadaParser();
}
void Parser::Options::UseYamadaParser() {
  impl_->UseYamadaParser();
}
void Parser::Options::NoDependencyParser() {
  impl_->NoDependencyParser();
}
void Parser::Options::SetModelPath(const char *model_path) {
  impl_->SetModelPath(model_path);
}
void Parser::Options::SetUserDictionary(const char *userdict_path) {
  impl_->SetUserDictionary(userdict_path);
}

const char *LastError() {
  return gLastErrorMessage;
}

}  // namespace milkcat
