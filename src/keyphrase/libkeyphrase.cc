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

class Keyphrase::Impl {
 public:
  static Keyphrase::Impl *New(Model *model);
  ~Impl();

  Iterator *Extract(const char *text);
  void Release(Iterator *it);

 private:
  PhraseExtractor *phrase_extractor_;
  PhraseRanker *phrase_ranker_;
  Parser *parser_;

  std::vector<Phrase *> keyphrases_;
  std::vector<Iterator *> iterator_pool_;
  int iterator_alloced_;

  const TrieTree *stopwords_;

  Impl();

  // Segment the text and save them into the document_ in it_impl
  void SegmentTextToDocument(const char *text, Document *document);
};

class Keyphrase::Iterator::Impl {
 public:
  ~Impl();
  Impl();

  bool End() {
    return keyphrases_pos_ >= keyphrases_.size();
  }

  void Next() { if (!End()) keyphrases_pos_++; }

  const char *phrase() { return keyphrases_[keyphrases_pos_]->PhraseString(); }
  double weight() { return keyphrases_[keyphrases_pos_]->weight();}

  std::vector<Phrase *> *keyphrases() { return &keyphrases_; }
  Document *document() { return document_; }
  utils::Pool<Phrase> *phrase_pool() { return &phrase_pool_; }

  void Reset() { keyphrases_pos_ = 0; }

 private:
  std::vector<Phrase *> keyphrases_;
  int keyphrases_pos_;

  Document *document_;
  utils::Pool<Phrase> phrase_pool_;


  friend class Keyphrase::Impl;
};

Keyphrase::Impl::Impl(): phrase_extractor_(NULL),
                         phrase_ranker_(NULL),
                         parser_(NULL),
                         stopwords_(NULL),
                         iterator_alloced_(0) {
}

Keyphrase::Impl::~Impl() {
  delete phrase_extractor_;
  phrase_extractor_ = NULL;

  delete phrase_ranker_;
  phrase_ranker_ = NULL;

  delete parser_;
  parser_ = NULL;
}

Keyphrase::Impl *Keyphrase::Impl::New(Model *model) {
  Keyphrase::Impl *self = new Keyphrase::Impl();
  global_status = Status::OK();

  self->phrase_extractor_ = new PhraseExtractor();
  self->phrase_ranker_ = PhraseRanker::New(model->impl(), &global_status);

  if (global_status.ok()) 
    self->stopwords_ = model->impl()->Stopword(&global_status);

  // Create the segmenter
  if (global_status.ok()) {
    self->parser_ = Parser::New(model, Parser::kSegmenter);
    if (self->parser_ == NULL) {
      global_status = Status::Info(LastError());
    }
  }

  if (global_status.ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

void Keyphrase::Impl::SegmentTextToDocument(const char *text,
                                            Document *document) {
  document->Clear();

  Parser::Iterator *it = parser_->Parse(text);
  while (!it->End()) {
    document->Add(it->word(), it->type());
    it->Next();
  }
  parser_->Release(it);
}

Keyphrase::Iterator *Keyphrase::Impl::Extract(const char *text) {
  Iterator *iterator;
  if (iterator_pool_.size() == 0) {
    ASSERT(iterator_alloced_ < 1024,
           "Too many Keyphrase::Iterator allocated without "
           "Keyphrase::Release.");
    iterator = new Iterator();
    iterator->impl()->document_ = new Document(stopwords_);
    iterator_alloced_++;
  } else {
    iterator = iterator_pool_.back();
    iterator_pool_.pop_back();
  }
  
  SegmentTextToDocument(text, iterator->impl()->document());
  
  // Extract phrases from the document
  phrase_extractor_->Extract(iterator->impl()->document(),
                             iterator->impl()->phrase_pool(),
                             iterator->impl()->keyphrases());

  // Rank the phrases by its weight
  phrase_ranker_->Rank(iterator->impl()->document(),
                       iterator->impl()->keyphrases());
  
  iterator->impl()->Reset();
  return iterator;
}

void Keyphrase::Impl::Release(Iterator *it) {
  iterator_pool_.push_back(it);
}

Keyphrase::Iterator::Impl::Impl():
    keyphrases_pos_(0),
    document_(NULL) {
}

Keyphrase::Iterator::Impl::~Impl() {
  delete document_;
  document_ = NULL;
}


Keyphrase *Keyphrase::New(Model *model) {
  Keyphrase *self = new Keyphrase();
  self->impl_ = Keyphrase::Impl::New(model);
  if (self->impl_) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

Keyphrase::Keyphrase(): impl_(NULL) {
}

Keyphrase::~Keyphrase() {
  delete impl_;
  impl_ = NULL;
}

Keyphrase::Iterator *Keyphrase::Extract(const char *text) {
  return impl_->Extract(text);
}

void Keyphrase::Release(Iterator *it) { impl_->Release(it); }

Keyphrase::Iterator::Iterator(): impl_(new Keyphrase::Iterator::Impl()) {
}

Keyphrase::Iterator::~Iterator() {
  delete impl_;
  impl_ = NULL;
}

bool Keyphrase::Iterator::End() { return impl_->End(); }
void Keyphrase::Iterator::Next() { impl_->Next(); }
const char *Keyphrase::Iterator::phrase() { return impl_->phrase(); }
double Keyphrase::Iterator::weight() { return impl_->weight(); }

}  // namespace milkcat
