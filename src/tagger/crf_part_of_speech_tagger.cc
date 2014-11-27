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
// crf_pos_tagger.cc --- Created at 2013-10-09
// crf_part_of_speech_tagger.cc --- Created at 2013-11-24
//

#include "tagger/crf_part_of_speech_tagger.h"

#include <string.h>
#include "common/milkcat_config.h"
#include "ml/sequence_feature_set.h"
#include "utils/utils.h"

namespace milkcat {


CRFPartOfSpeechTagger::CRFPartOfSpeechTagger(const CRFModel *model) {
  sequence_feature_set_ = new SequenceFeatureSet();
  crf_tagger_ = new CRFTagger(model);
}

CRFPartOfSpeechTagger::CRFPartOfSpeechTagger(): crf_tagger_(NULL),
                                                sequence_feature_set_(NULL) {
}

CRFPartOfSpeechTagger::~CRFPartOfSpeechTagger() {
  delete sequence_feature_set_;
  sequence_feature_set_ = NULL;

  delete crf_tagger_;
  crf_tagger_ = NULL;
}

void CRFPartOfSpeechTagger::TagRange(
    PartOfSpeechTagInstance *part_of_speech_tag_instance,
    TermInstance *term_instance,
    int begin,
    int end) {
  char buff[kFeatureLengthMax];

  // Prepares the `sequence_feature_set_` for tagging
  sequence_feature_set_->set_size(term_instance->size());
  for (int idx = 0; idx < term_instance->size(); ++idx) {
    int type = term_instance->term_type_at(idx);
    const char *word = term_instance->term_text_at(idx);
    int length = strlen(word);  

    FeatureSet *feature_set = sequence_feature_set_->at_index(idx);
    feature_set->Clear();

    switch (type) {
      case Parser::kChineseWord:
        // term itself
        feature_set->Add(word);
        strlcpy(buff, word, 4);
        feature_set->Add(buff);
        feature_set->Add(word + length - 3);
        sprintf(buff, "%d", length / 3);
        feature_set->Add(buff);
        break;

      case Parser::kEnglishWord:
      case Parser::kSymbol:
        feature_set->Add("A");
        feature_set->Add("A");
        feature_set->Add("A");
        feature_set->Add("1");
        break;

      case Parser::kNumber:
        feature_set->Add("1");
        feature_set->Add("1");
        feature_set->Add("1");
        feature_set->Add("1");
        break;

      default:
        feature_set->Add(".");
        feature_set->Add(".");
        feature_set->Add(".");
        feature_set->Add("1");
        break;
    }  
  }

  crf_tagger_->TagRange(sequence_feature_set_, begin, end);
  for (size_t i = 0; i < end - begin; ++i) {
    part_of_speech_tag_instance->set_value_at(
        i,
        crf_tagger_->GetTagText(crf_tagger_->y(i)));
  }
  part_of_speech_tag_instance->set_size(end - begin);
}

}  // namespace milkcat
