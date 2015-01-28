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

#include "segmenter/out_of_vocabulary_word_recognition.h"

#include <stdio.h>
#include <string.h>
#include "libmilkcat.h"
#include "common/model_impl.h"
#include "common/reimu_trie.h"
#include "include/milkcat.h"
#include "segmenter/crf_segmenter.h"
#include "tokenizer/token_instance.h"
#include "util/util.h"

namespace milkcat {

OutOfVocabularyWordRecognition *OutOfVocabularyWordRecognition::New(
    Model::Impl *model_factory,
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

void OutOfVocabularyWordRecognition::GetOOVProperties(
    TermInstance *term_instance) {
  for (int i = 0; i < term_instance->size(); ++i) 
    oov_properties_[i] = kNoRecognize;

  for (int i = 0; i < term_instance->size(); ++i) {
    int token_number = term_instance->token_number_at(i);
    int term_type = term_instance->term_type_at(i);
    if (token_number > 1) {
      continue;
    } else if (term_type != Parser::kChineseWord) {
      continue;
    } else {
      const char *term_text = term_instance->term_text_at(i);
      int oov_property = oov_property_->Get(term_text, -1);
      if (oov_property < 0) {
        oov_properties_[i] = kDoRecognize;
      } else {
        switch (oov_property) {
          case kOOVBeginOfWord:
            oov_properties_[i] = kDoRecognize;
            if (i < term_instance->size() - 1) {
              oov_properties_[i + 1] = kDoRecognize;
            }
            break;

          case kOOVEndOfWord:
            oov_properties_[i] = kDoRecognize;
            if (i > 0 && oov_properties_[i - 1] != kNeverRecognize) {
              oov_properties_[i - 1] = kDoRecognize;
            }
            break;

          case kOOVFilteredWord:
            oov_properties_[i] = kNeverRecognize;
            break;
        }
      }
    }
  }
}

void OutOfVocabularyWordRecognition::Recognize(
    TermInstance *term_instance,
    TermInstance *in_term_instance,
    TokenInstance *in_token_instance) {
  int current_token = 0;
  int current_term = 0;
  int term_token_number;
  int oov_begin_token = 0;
  int oov_token_num = 0;

  GetOOVProperties(in_term_instance);

  for (int i = 0; i < in_term_instance->size(); ++i) {
    term_token_number = in_term_instance->token_number_at(i);

    if (oov_properties_[i] == kDoRecognize) {
      oov_token_num++;

    } else {
      // Recognize token from oov_begin_token to current_token
      if (oov_token_num > 1) {
        RecognizeRange(in_token_instance, oov_begin_token, current_token);

        for (int j = 0; j < term_instance_->size(); ++j) {
          CopyTermValue(term_instance, current_term, term_instance_, j);
          current_term++;
        }
      } else if (oov_token_num == 1) {
        CopyTermValue(term_instance, current_term, in_term_instance, i - 1);
        current_term++;
      }

      CopyTermValue(term_instance, current_term, in_term_instance, i);
      oov_begin_token = current_token + term_token_number;
      current_term++;
      oov_token_num = 0;
    }

    current_token += term_token_number;
  }

  // Recognize remained tokens
  if (oov_token_num > 1) {
    RecognizeRange(in_token_instance, oov_begin_token, current_token);
    for (int j = 0; j < term_instance_->size(); ++j) {
      CopyTermValue(term_instance, current_term, term_instance_, j);
      current_term++;
    }
  } else if (oov_token_num == 1) {
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
