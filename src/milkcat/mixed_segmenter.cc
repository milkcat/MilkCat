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
// mixed_segmenter.cc --- Created at 2013-11-25
//

#include "milkcat/mixed_segmenter.h"
#include "milkcat/bigram_segmenter.h"
#include "milkcat/out_of_vocabulary_word_recognition.h"
#include "milkcat/term_instance.h"
#include "milkcat/token_instance.h"

namespace milkcat {

MixedSegmenter::MixedSegmenter():
    bigram_result_(NULL),
    bigram_(NULL),
    oov_recognizer_(NULL) {
}

MixedSegmenter *MixedSegmenter::New(ModelFactory *model_factory, 
                                    Status *status) {
  MixedSegmenter *self = new MixedSegmenter();
  self->bigram_ = BigramSegmenter::New(model_factory, true, status);

  if (status->ok()) {
    self->oov_recognizer_ = OutOfVocabularyWordRecognition::New(model_factory,
                                                                status);    
  }
  if (status->ok()) self->bigram_result_ = new TermInstance();

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

MixedSegmenter::~MixedSegmenter() {
  delete bigram_result_;
  bigram_result_ = NULL;

  delete bigram_;
  bigram_ = NULL;

  delete oov_recognizer_;
  oov_recognizer_ = NULL;
}

void MixedSegmenter::Segment(TermInstance *term_instance,
                             TokenInstance *token_instance) {
  bigram_->Segment(bigram_result_, token_instance);
  oov_recognizer_->Recognize(term_instance, bigram_result_, token_instance);
}

}  // namespace milkcat
