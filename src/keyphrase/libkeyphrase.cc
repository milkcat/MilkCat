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
// keyphrase.h --- Created at 2014-04-19
//

#include "include/milkcat.h"

#include <vector>
#include "common/model_factory.h"
#include "common/trie_tree.h"
#include "keyphrase/document.h"
#include "keyphrase/phrase_extractor.h"
#include "keyphrase/phrase_ranker.h"
#include "milkcat/libmilkcat.h"
#include "utils/pool.h"
#include "utils/status.h"

namespace milkcat {

class Keyphrase {
 public:
  static Keyphrase *New(milkcat_model_t *model, Status *status);
  ~Keyphrase();

  // Extract keyphrase from text
  void Extract(const char *text);

  // Get next keyphrase and step the current cursor
  Phrase *NextKeyPhrase() {
    if (keyphrases_pos_ < keyphrases_.size()) {
      return keyphrases_[keyphrases_pos_++];
    } else {
      return NULL;
    }
  }

 private:
  PhraseExtractor *phrase_extractor_;
  PhraseRanker *phrase_ranker_;
  milkcat_t *analyzer_;
  milkcat_cursor_t *cursor_;

  Document *document_;
  std::vector<Phrase *> keyphrases_;
  int keyphrases_pos_;

  Keyphrase();

  // Segment the text and save them into the document_
  void SegmentTextToDocument(const char *text);
};

Keyphrase::Keyphrase(): phrase_extractor_(NULL),
                        phrase_ranker_(NULL),
                        analyzer_(NULL),
                        cursor_(NULL),
                        document_(NULL),
                        keyphrases_pos_(0) {
}

Keyphrase::~Keyphrase() {
  delete phrase_extractor_;
  phrase_extractor_ = NULL;

  delete phrase_ranker_;
  phrase_ranker_ = NULL;

  milkcat_destroy(analyzer_);
  analyzer_ = NULL;

  milkcat_cursor_destroy(cursor_);
  cursor_ = NULL;

  delete document_;
  document_ = NULL;
}

Keyphrase *Keyphrase::New(milkcat_model_t *model, Status *status) {
  Keyphrase *self = new Keyphrase();

  // Extract model factory instance from model
  ModelFactory *model_factory = model->model_factory;

  self->phrase_extractor_ = new PhraseExtractor();
  self->phrase_ranker_ = PhraseRanker::New(model_factory, status);

  const TrieTree *stopword = NULL;
  if (status->ok()) stopword = model_factory->Stopword(status);
  if (status->ok()) self->document_ = new Document(stopword);

  // Create the segmenter
  if (status->ok()) {
    self->analyzer_ = milkcat_new(model, DEFAULT_SEGMENTER);
    self->cursor_ = milkcat_cursor_new();
    if (self->analyzer_ == NULL) {
      *status = Status::Info(milkcat_last_error());
    }
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

void Keyphrase::SegmentTextToDocument(const char *text) {
  document_->Clear();

  milkcat_item_t item;
  milkcat_analyze(analyzer_, cursor_, text);
  while (milkcat_cursor_get_next(cursor_, &item)) {
    document_->Add(item.word, item.word_type);
  }
}

void Keyphrase::Extract(const char *text) {
  SegmentTextToDocument(text);

  phrase_extractor_->Extract(*document_, &keyphrases_);
  phrase_ranker_->Rank(*document_, &keyphrases_);

  keyphrases_pos_ = 0;
}

}  // namespace milkcat

struct milkcat_keyphrase_t {
  milkcat::Keyphrase *keyphrase;
  keyphrase_item_t item;
};

milkcat_keyphrase_t *milkcat_keyphrase_new(milkcat_model_t *model, int method) {
  milkcat_keyphrase_t *keyphrase_handler = new milkcat_keyphrase_t;

  keyphrase_handler->keyphrase = milkcat::Keyphrase::New(
      model,
      &milkcat::global_status);
  
  if (milkcat::global_status.ok()) {
    return keyphrase_handler;
  } else {
    delete keyphrase_handler;
    return NULL;
  }
}

keyphrase_item_t *
milkcat_keyphrase_extract(milkcat_keyphrase_t *keyphrase, const char *text) {
  if (text) {
    keyphrase->keyphrase->Extract(text);
  }

  milkcat::Phrase *phrase = keyphrase->keyphrase->NextKeyPhrase();

  if (phrase) {
    keyphrase->item.keyphrase = phrase->PhraseString();
    keyphrase->item.weight = phrase->weight();
    return &(keyphrase->item);
  } else {
    return NULL;
  }
}

void milkcat_keyphrase_destroy(milkcat_keyphrase_t *keyphrase) {
  delete keyphrase->keyphrase;
  delete keyphrase;
}