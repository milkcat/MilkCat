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
// tag_segmenter.cc --- Created at 2013-02-12
// crf_segmenter.cc --- Created at 2013-08-17
//


#include <stdio.h>
#include <string.h>
#include <string>
#include "utils/utils.h"
#include "milkcat/crf_segmenter.h"
#include "milkcat/libmilkcat.h"
#include "milkcat/term_instance.h"
#include "milkcat/token_instance.h"
#include "milkcat/feature_extractor.h"

namespace milkcat {

class SegmentFeatureExtractor: public FeatureExtractor {
 public:
  void set_token_instance(const TokenInstance *token_instance) {
    token_instance_ = token_instance;
  }

  size_t size() const { return token_instance_->size(); }

  void ExtractFeatureAt(size_t position,
                        char (*feature_list)[kFeatureLengthMax],
                        int list_size) {
    assert(list_size == 1);
    int token_type = token_instance_->token_type_at(position);
    if (token_type == TokenInstance::kChineseChar) {
      utils::strlcpy(feature_list[0],
                     token_instance_->token_text_at(position),
                     kFeatureLengthMax);
    } else {
      utils::strlcpy(feature_list[0], "，", kFeatureLengthMax);
    }
  }

 private:
  const TokenInstance *token_instance_;
};

CRFSegmenter *CRFSegmenter::New(ModelFactory *model_factory, Status *status) {
  CRFSegmenter *self = new CRFSegmenter();
  const CRFModel *model = model_factory->CRFSegModel(status);
  
  if (status->ok()) {
    self->crf_tagger_ = new CRFTagger(model);
    self->feature_extractor_ = new SegmentFeatureExtractor();

    // Get the tag's value in CRF++ model
    self->S = self->crf_tagger_->GetTagId("S");
    self->B = self->crf_tagger_->GetTagId("B");
    self->B1 = self->crf_tagger_->GetTagId("B1");
    self->B2 = self->crf_tagger_->GetTagId("B2");
    self->M = self->crf_tagger_->GetTagId("M");
    self->E = self->crf_tagger_->GetTagId("E");

    if (self->S < 0 || self->B < 0 || self->B1 < 0 || self->B2 < 0 ||
        self->M < 0 || self->E < 0) {
      *status = Status::Corruption(
        "bad CRF++ segmenter model, unable to find S, B, B1, B2, M, E tag.");
    }    
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

CRFSegmenter::~CRFSegmenter() {
  delete feature_extractor_;
  feature_extractor_ = NULL;

  delete crf_tagger_;
  crf_tagger_ = NULL;
}

CRFSegmenter::CRFSegmenter(): crf_tagger_(NULL),
                              feature_extractor_(NULL) {}

void CRFSegmenter::SegmentRange(TermInstance *term_instance,
                                TokenInstance *token_instance,
                                int begin,
                                int end) {
  std::string buffer;

  feature_extractor_->set_token_instance(token_instance);
  crf_tagger_->TagRange(feature_extractor_, begin, end, S, S);

  int tag_id;
  int term_count = 0;
  size_t i = 0;
  int token_count = 0;
  int term_type;
  for (i = 0; i < end - begin; ++i) {
    token_count++;
    buffer.append(token_instance->token_text_at(begin + i));

    tag_id = crf_tagger_->GetTagAt(i);
    if (tag_id == S || tag_id == E) {
      if (tag_id == S) {
        term_type = TokenTypeToTermType(
          token_instance->token_type_at(begin + i));
      } else {
        term_type = TermInstance::kChineseWord;
      }

      term_instance->set_value_at(term_count,
                                  buffer.c_str(),
                                  token_count,
                                  term_type);
      term_count++;
      token_count = 0;
      buffer.clear();
    }
  }

  if (!buffer.empty()) {
    term_instance->set_value_at(term_count,
                                buffer.c_str(),
                                token_count,
                                TermInstance::kChineseWord);
    term_count++;
  }

  term_instance->set_size(term_count);
}

}  // namespace milkcat
