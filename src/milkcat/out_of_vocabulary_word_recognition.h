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
// named_entity_recognitioin.h --- Created at 2013-08-26
// out_of_vocabulary_word_recognitioin.h --- Created at 2013-09-03
//


#ifndef SRC_MILKCAT_OUT_OF_VOCABULARY_WORD_RECOGNITION_H_
#define SRC_MILKCAT_OUT_OF_VOCABULARY_WORD_RECOGNITION_H_

#include <stdio.h>
#include "utils/utils.h"
#include "milkcat/darts.h"
#include "milkcat/crf_segmenter.h"
#include "milkcat/part_of_speech_tag_instance.h"
#include "milkcat/crf_model.h"
#include "milkcat/trie_tree.h"
#include "milkcat/token_instance.h"

namespace milkcat {

class ModelFactory;

class OutOfVocabularyWordRecognition {
 public:
  static OutOfVocabularyWordRecognition *New(ModelFactory *model_factory,
                                             Status *status);
  ~OutOfVocabularyWordRecognition();
  void Process(TermInstance *term_instance,
               TermInstance *in_term_instance,
               TokenInstance *in_token_instance);

  static const int kOOVBeginOfWord = 1;
  static const int kOOVFilteredWord = 2;

 private:
  TermInstance *term_instance_;
  CRFSegmenter *crf_segmenter_;
  const TrieTree *oov_property_;

  OutOfVocabularyWordRecognition();

  void RecognizeRange(TokenInstance *token_instance, int begin, int end);

  void CopyTermValue(TermInstance *dest_term_instance,
                     int dest_postion,
                     TermInstance *src_term_instance,
                     int src_position);

  DISALLOW_COPY_AND_ASSIGN(OutOfVocabularyWordRecognition);
};

}  // namespace milkcat

#endif  // SRC_MILKCAT_OUT_OF_VOCABULARY_WORD_RECOGNITION_H_
