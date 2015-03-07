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
// model_factory.h --- Created at 2014-02-03
// libmilkcat.h --- Created at 2014-02-06
//

#ifndef SRC_PARSER_LIBMILKCAT_H_
#define SRC_PARSER_LIBMILKCAT_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <string>
#include <vector>
#include "common/milkcat_config.h"
#include "include/milkcat.h"
#include "segmenter/segmenter.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tagger.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "parser/dependency_parser.h"
#include "parser/tree_instance.h"
#include "tokenizer/tokenizer.h"
#include "util/encoding.h"
#include "util/mutex.h"
#include "util/util.h"
#include "util/status.h"
#include "util/readable_file.h"

namespace milkcat {
  
// The global error message
extern char gLastErrorMessage[kLastErrorStringMax];

const int kTokenizerMask = 0x0000000f;
const int kSegmenterMask = 0x00000ff0;
const int kPartOfSpeechTaggerMask = 0x000ff000;
const int kParserMask = 0x00f00000;

// A factory function to create tokenizers
Tokenization *TokenizerFactory(int tokenizer_id);

// A factory function to create segmenters. On success, return the instance of
// Segmenter, on failed, set status != Status::OK()
Segmenter *SegmenterFactory(Model::Impl *factory,
                            int segmenter_id,
                            Status *status);

// A factory function to create part-of-speech taggers. On success, return the
// instance of part-of-speech tagger, on failed, set status != Status::OK()
PartOfSpeechTagger *PartOfSpeechTaggerFactory(Model::Impl *factory,
                                              int part_of_speech_tagger_id,
                                              Status *status);


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

class Parser::Impl {
 public:
  static Impl *New(const Options &options, Model *model);
  ~Impl();

  void Predict(Iterator *iterator, const char *text);

  Segmenter *segmenter() const { return segmenter_; }
  PartOfSpeechTagger *part_of_speech_tagger() const {
    return part_of_speech_tagger_;
  }
  DependencyParser *dependency_parser() const {
    return dependency_parser_;
  }

  // Get the segmenter for the parser
  Segmenter *segmenter() { return segmenter_; }

 private:
  Impl();

  // Converts term_instance to GBK encoding
  void ConvertToGBKTermInstance(TermInstance *term_instance);

  Tokenization *tokenizer_;
  Segmenter *segmenter_;
  PartOfSpeechTagger *part_of_speech_tagger_;
  DependencyParser *dependency_parser_;
  Model::Impl *model_impl_;
  bool own_model_;

  // These fields are used when using gbk encoding
  bool use_gbk_;
  char *utf8_buffer_;
  int utf8_buffersize_;
  Encoding *encoding_;
};

class Parser::Options::Impl {
 public:
  Impl();

  void UseGBK() {
    use_gbk_ = true;
  }
  void UseUTF8() {
    use_gbk_ = false;
  }

  void UseMixedSegmenter() {
    segmenter_type_ = kMixedSegmenter;
  }
  void UseCRFSegmenter() {
    segmenter_type_ = kCrfSegmenter;
  }
  void UseUnigramSegmenter() {
    segmenter_type_ = kUnigramSegmenter;
  }
  void UseBigramSegmenter() {
    segmenter_type_ = kBigramSegmenter;
  }

  void UseMixedPOSTagger() {
    tagger_type_ = kMixedTagger;
  }
  void UseHMMPOSTagger() {
    tagger_type_ = kHmmTagger;
  }
  void UseCRFPOSTagger() {
    tagger_type_ = kCrfTagger;
  }
  void NoPOSTagger() {
    tagger_type_ = kNoTagger;
  }

  void UseYamadaParser() {
    if (tagger_type_ == kNoTagger) tagger_type_ = kMixedTagger;
    parser_type_ = kBeamYamadaParser;
  }
  void UseBeamYamadaParser() {
    if (tagger_type_ == kNoTagger) tagger_type_ = kMixedTagger;
    parser_type_ = kYamadaParser;
  }
  void NoDependencyParser() {
    parser_type_ = kNoParser;
  }

  // Get the type value of current setting
  int TypeValue() const {
    return segmenter_type_ | tagger_type_ | parser_type_;
  }
  bool use_gbk() const { return use_gbk_; }

private:
  int segmenter_type_;
  int tagger_type_;
  int parser_type_;
  bool use_gbk_;
};

// Represents the parsing result of a sentence
class SentenceInstance {
 public:
  SentenceInstance() {
    token_instance_ = new TokenInstance();
    term_instance_ = new TermInstance();
    part_of_speech_tag_instance_ = new PartOfSpeechTagInstance();
    tree_instance_ = new TreeInstance();
  }

  ~SentenceInstance() {
    delete token_instance_;
    token_instance_ = NULL;

    delete term_instance_;
    term_instance_ = NULL;

    delete part_of_speech_tag_instance_;
    part_of_speech_tag_instance_ = NULL;

    delete tree_instance_;
    tree_instance_ = NULL;
  }

  TokenInstance *token_instance() const {
    return token_instance_;
  }
  TermInstance *term_instance() const {
    return term_instance_;
  }
  PartOfSpeechTagInstance *part_of_speech_tag_instance() const {
    return part_of_speech_tag_instance_;
  }
  TreeInstance *tree_instance() const {
    return tree_instance_;
  }

 private:
  TokenInstance *token_instance_;
  TermInstance *term_instance_;
  PartOfSpeechTagInstance *part_of_speech_tag_instance_;
  TreeInstance *tree_instance_;
};

// Cursor class save the internal state of the analyzing result, such as
// the current word and current sentence.
class Parser::Iterator::Impl {
 public:
  Impl();
  ~Impl();

  // Resets this iterator
  void Reset(int sentence_num, bool have_postagger, bool have_parser) {
    sentence_num_ = sentence_num;
    end_ = (sentence_num_ == 0);
    have_postagger_ = have_postagger;
    have_parser_ = have_parser;
    current_sentence_ = 0;
    current_idx_ = 0;
    begin_ = false;
  }

  // These function return the data of current position
  const char *word() const {
    if (end_ || begin_ == false) return "";
    return sentence_[current_sentence_]->term_instance()
                                       ->term_text_at(current_idx_);
  }
  const char *part_of_speech_tag() const {
    if (end_ || begin_ == false) return "";
    if (have_postagger_) {
      return sentence_[current_sentence_]->part_of_speech_tag_instance()
                                         ->part_of_speech_tag_at(current_idx_);
    } else {
      return "none";
    }
  }
  int type() const {
    if (end_ || begin_ == false) return 0;
    return sentence_[current_sentence_]->term_instance()
                                       ->term_type_at(current_idx_);
  }
  int head() const {
    if (end_ || begin_ == false) return 0;
    if (have_parser_) {
      return sentence_[current_sentence_]->tree_instance()
                                         ->head_node_at(current_idx_);
    } else {
      return 0;
    }
  }
  const char *dependency_label() const {
    if (end_ || begin_ == false) return "";
    if (have_parser_) {
      return sentence_[current_sentence_]->tree_instance()
                                         ->dependency_type_at(current_idx_);
    } else {
      return "none";
    }
  }
  bool is_begin_of_sentence() const {
    return current_idx_ == 0;
  }

  SentenceInstance *sentence(int idx) {
    while (idx >= sentence_.size()) {
      sentence_.push_back(new SentenceInstance());
    }
    return sentence_[idx];
  }

  // Move the cursor to next position, if end of text is reached
  // set end() to true
  bool Next() {
    if (end_) return false;
    if (begin_ == false) {
      begin_ = true;
      return true;
    }
    SentenceInstance *cnt_sentence = sentence(current_sentence_);
    if (current_idx_ >= cnt_sentence->term_instance()->size() - 1) {
      current_idx_ = 0;
      ++current_sentence_;
    } else {
      ++current_idx_;
    }
    if (current_sentence_ >= sentence_num_) {
      end_ = true;
      return false;
    } else {
      return true;
    }
  }

 private:
  int sentence_num_;
  int current_sentence_;
  int current_idx_;
  bool begin_;
  bool end_;

  bool have_postagger_;
  bool have_parser_;

  std::vector<SentenceInstance *> sentence_;
};

}  // namespace milkcat

#endif  // SRC_PARSER_LIBMILKCAT_H_
