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

#ifndef SRC_MILKCAT_LIBMILKCAT_H_
#define SRC_MILKCAT_LIBMILKCAT_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <string>
#include <vector>
#include "common/milkcat_config.h"
#include "include/milkcat.h"
#include "milkcat/segmenter.h"
#include "milkcat/part_of_speech_tagger.h"
#include "milkcat/tokenizer.h"
#include "milkcat/term_instance.h"
#include "milkcat/part_of_speech_tag_instance.h"
#include "utils/mutex.h"
#include "utils/utils.h"
#include "utils/status.h"
#include "utils/readable_file.h"


namespace milkcat {
  
// The global status
extern milkcat::Status global_status;

class ModelFactory;
class Cursor;

}  // namespace milkcat

struct milkcat_model_t {
  milkcat::ModelFactory *model_factory;
};

struct milkcat_t {
  milkcat_model_t *model;
  milkcat_item_t item;
  milkcat::Cursor *cursor;
  milkcat::Segmenter *segmenter;
  milkcat::PartOfSpeechTagger *part_of_speech_tagger;
};

namespace milkcat {

const int kTokenizerMask = 0x0000000f;
const int kSegmenterMask = 0x00000ff0;
const int kPartOfSpeechTaggerMask = 0x000ff000;

// A factory function to create tokenizers
Tokenization *TokenizerFactory(int tokenizer_id);

// A factory function to create segmenters. On success, return the instance of
// Segmenter, on failed, set status != Status::OK()
Segmenter *SegmenterFactory(ModelFactory *factory,
                           int segmenter_id,
                           Status *status);

// A factory function to create part-of-speech taggers. On success, return the
// instance of part-of-speech tagger, on failed, set status != Status::OK()
PartOfSpeechTagger *PartOfSpeechTaggerFactory(ModelFactory *factory,
                                              int part_of_speech_tagger_id,
                                              Status *status);


// Cursor class save the internal state of the analyzing result, such as
// the current word and current sentence.
class Cursor {
 public:
  explicit Cursor();
  ~Cursor();

  // Start to scan a text and use this->analyzer_ to analyze it
  // the result saved in its current state, use MoveToNext() to
  // iterate.
  void Scan(const char *text);

  // Move the cursor to next position, if end of text is reached
  // set end() to true
  void MoveToNext();

  // These function return the data of current position
  const char *word() const {
    return term_instance_->term_text_at(current_position_);
  }
  const char *part_of_speech_tag() const {
    if (analyzer_->part_of_speech_tagger != NULL)
      return part_of_speech_tag_instance_->part_of_speech_tag_at(
          current_position_);
    else
      return "NONE";
  }
  const int word_type() const {
    return term_instance_->term_type_at(current_position_);
  }

  // If reaches the end of text
  bool end() const { return end_; }

  milkcat_t *analyzer() const { return analyzer_; }
  void set_analyzer(milkcat_t *analyzer) {
    analyzer_ = analyzer;
  }

 private:
  milkcat_t *analyzer_;

  Tokenization *tokenizer_;
  TokenInstance *token_instance_;
  TermInstance *term_instance_;
  PartOfSpeechTagInstance *part_of_speech_tag_instance_;

  int sentence_length_;
  int current_position_;
  bool end_;
};

}  // namespace milkcat

#endif  // SRC_MILKCAT_LIBMILKCAT_H_
