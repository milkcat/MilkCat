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
// mixed_segmenter.h --- Created at 2013-11-25
//

#ifndef SRC_SEGMENTER_MIXED_SEGMENTER_H_
#define SRC_SEGMENTER_MIXED_SEGMENTER_H_

#include "include/milkcat.h"
#include "common/static_hashtable.h"
#include "ml/crf_model.h"
#include "segmenter/segmenter.h"
#include "utils/utils.h"

namespace milkcat {

class OutOfVocabularyWordRecognition;
class BigramSegmenter;
class TermInstance;
class TokenInstance;

// Mixed Bigram segmenter and CRF Segmenter of OOV recognition
class MixedSegmenter: public Segmenter {
 public:
  ~MixedSegmenter();

  static MixedSegmenter *New(Model::Impl *model_factory, Status *status);

  // Segment a token instance into term instance
  void Segment(TermInstance *term_instance, TokenInstance *token_instance);

 private:
  TermInstance *bigram_result_;
  BigramSegmenter *bigram_;
  OutOfVocabularyWordRecognition *oov_recognizer_;

  MixedSegmenter();
};

}  // namespace milkcat

#endif  // SRC_SEGMENTER_MIXED_SEGMENTER_H_
