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

#include "milkcat/crf_part_of_speech_tagger.h"
#include <string.h>
#include "utils/utils.h"
#include "milkcat/feature_extractor.h"
#include "milkcat/milkcat_config.h"

namespace milkcat {

void PartOfSpeechFeatureExtractor::ExtractFeatureAt(
    size_t position,
    char (*feature_list)[kFeatureLengthMax],
    int list_size) {
  assert(list_size == 3);
  int term_type = term_instance_->term_type_at(position);
  const char *term_text = term_instance_->term_text_at(position);
  size_t term_length = strlen(term_text);

  switch (term_type) {
   case TermInstance::kChineseWord:
    // term itself
    utils::strlcpy(feature_list[0], term_text, kFeatureLengthMax);

    // first character of the term
    utils::strlcpy(feature_list[1], term_text, 4);

    // last character of the term
    utils::strlcpy(feature_list[2],
                   term_text + term_length - 3,
                   kFeatureLengthMax);
    break;

   case TermInstance::kEnglishWord:
   case TermInstance::kSymbol:
    utils::strlcpy(feature_list[0], "A", kFeatureLengthMax);
    utils::strlcpy(feature_list[1], "A", kFeatureLengthMax);
    utils::strlcpy(feature_list[2], "A", kFeatureLengthMax);
    break;

   case TermInstance::kNumber:
    utils::strlcpy(feature_list[0], "1", kFeatureLengthMax);
    utils::strlcpy(feature_list[1], "1", kFeatureLengthMax);
    utils::strlcpy(feature_list[2], "1", kFeatureLengthMax);
    break;

   default:
    utils::strlcpy(feature_list[0], ".", kFeatureLengthMax);
    utils::strlcpy(feature_list[1], ".", kFeatureLengthMax);
    utils::strlcpy(feature_list[2], ".", kFeatureLengthMax);
    break;
  }
}


CRFPartOfSpeechTagger::CRFPartOfSpeechTagger(const CRFModel *model) {
  feature_extractor_ = new PartOfSpeechFeatureExtractor();
  crf_tagger_ = new CRFTagger(model);
}

CRFPartOfSpeechTagger::CRFPartOfSpeechTagger(): crf_tagger_(NULL),
                                                feature_extractor_(NULL) {
}

CRFPartOfSpeechTagger::~CRFPartOfSpeechTagger() {
  delete feature_extractor_;
  feature_extractor_ = NULL;

  delete crf_tagger_;
  crf_tagger_ = NULL;
}

void CRFPartOfSpeechTagger::TagRange(
    PartOfSpeechTagInstance *part_of_speech_tag_instance,
    TermInstance *term_instance,
    int begin,
    int end) {
  feature_extractor_->set_term_instance(term_instance);
  crf_tagger_->TagRange(feature_extractor_, begin, end);
  for (size_t i = 0; i < end - begin; ++i) {
    part_of_speech_tag_instance->set_value_at(
        i,
        crf_tagger_->GetTagText(crf_tagger_->GetTagAt(i)));
  }
  part_of_speech_tag_instance->set_size(end - begin);
}

}  // namespace milkcat
