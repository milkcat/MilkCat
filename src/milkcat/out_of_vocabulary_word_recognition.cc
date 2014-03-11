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
// named_entity_recognitioin.cc --- Created at 2013-08-26
// out_of_vocabulary_word_recognitioin.cc --- Created at 2013-09-03
//

#include "milkcat/out_of_vocabulary_word_recognition.h"
#include <stdio.h>
#include <string.h>
#include "milkcat/crf_segmenter.h"
#include "milkcat/darts.h"
#include "milkcat/libmilkcat.h"
#include "milkcat/token_instance.h"
#include "utils/utils.h"

namespace milkcat {

OutOfVocabularyWordRecognition *OutOfVocabularyWordRecognition::New(
    ModelFactory *model_factory,
    Status *status) {
  OutOfVocabularyWordRecognition *self = new OutOfVocabularyWordRecognition();
  self->crf_segmenter_ = CRFSegmenter::New(model_factory, status);

  if (status->ok()) {
    self->term_instance_ = new TermInstance();
    self->oov_property_ = model_factory->OOVProperty(status);
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

OutOfVocabularyWordRecognition::~OutOfVocabularyWordRecognition() {
  delete crf_segmenter_;
  crf_segmenter_ = NULL;

  delete term_instance_;
  term_instance_ = NULL;
}

OutOfVocabularyWordRecognition::OutOfVocabularyWordRecognition():
    term_instance_(NULL),
    crf_segmenter_(NULL) {
}

void OutOfVocabularyWordRecognition::Process(TermInstance *term_instance,
                                             TermInstance *in_term_instance,
                                             TokenInstance *in_token_instance) {
  int ner_begin_token = 0;
  int current_token = 0;
  int current_term = 0;
  int term_token_number;
  int current_token_type;
  int ner_term_number = 0;
  const char *term_str;
  bool oov_flag = false,
       next_oov_flag = false;

  for (size_t i = 0; i < in_term_instance->size(); ++i) {
    term_token_number = in_term_instance->token_number_at(i);
    current_token_type = in_token_instance->token_type_at(current_token);
    term_str = in_term_instance->term_text_at(i);
    // printf("%d\n", ner_term_number);

    if (term_token_number > 1) {
      if (next_oov_flag == true) {
        oov_flag = true;
        next_oov_flag = false;
      } else {
        oov_flag = false;
      }
    } else if (current_token_type != TokenInstance::kChineseChar) {
      oov_flag = false;
      next_oov_flag = false;
    } else {
      int oov_property = oov_property_->Search(term_str);
      if (oov_property == kOOVBeginOfWord) {
        next_oov_flag = true;
        oov_flag = true;
      } else if (oov_property == kOOVFilteredWord) {
        oov_flag = false;
        next_oov_flag = false;
      } else {
        oov_flag = true;
        next_oov_flag = false;
      }
    }

    if (oov_flag) {
      ner_term_number++;

    } else {
      // Recognize token from ner_begin_token to current_token
      if (ner_term_number > 1) {
        RecognizeRange(in_token_instance, ner_begin_token, current_token);

        for (size_t j = 0; j < term_instance_->size(); ++j) {
          CopyTermValue(term_instance, current_term, term_instance_, j);
          current_term++;
        }
      } else if (ner_term_number == 1) {
        CopyTermValue(term_instance, current_term, in_term_instance, i - 1);
        current_term++;
      }

      CopyTermValue(term_instance, current_term, in_term_instance, i);
      ner_begin_token = current_token + term_token_number;
      current_term++;
      ner_term_number = 0;
    }

    current_token += term_token_number;
  }

  // Recognize remained tokens
  if (ner_term_number > 1) {
    RecognizeRange(in_token_instance, ner_begin_token, current_token);
    for (size_t j = 0; j < term_instance_->size(); ++j) {
      CopyTermValue(term_instance, current_term, term_instance_, j);
      current_term++;
    }
    ner_begin_token = current_token + term_token_number;
  } else if (ner_term_number == 1) {
    CopyTermValue(term_instance,
                  current_term,
                  in_term_instance,
                  in_term_instance->size() - 1);
    current_term++;
  }

  term_instance->set_size(current_term);
}

void OutOfVocabularyWordRecognition::CopyTermValue(
    TermInstance *dest_term_instance,
    int dest_postion,
    TermInstance *src_term_instance,
    int src_position)  {

  dest_term_instance->set_value_at(
      dest_postion,
      src_term_instance->term_text_at(src_position),
      src_term_instance->token_number_at(src_position),
      src_term_instance->term_type_at(src_position),
      src_term_instance->term_id_at(src_position));
}

void OutOfVocabularyWordRecognition::RecognizeRange(
    TokenInstance *token_instance,
    int begin,
    int end) {
  crf_segmenter_->SegmentRange(term_instance_, token_instance, begin, end);
}

}  // namespace milkcat
