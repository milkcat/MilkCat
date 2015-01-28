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
// term_pos_tag_instance.h
// part_of_speech_tag_instance.h --- Created at 2013-10-20
//

#ifndef SRC_TAGGER_PART_OF_SPEECH_TAG_INSTANCE_H_
#define SRC_TAGGER_PART_OF_SPEECH_TAG_INSTANCE_H_

#include <assert.h>
#include "common/instance_data.h"
#include "segmenter/term_instance.h"
#include "util/util.h"

namespace milkcat {

class PartOfSpeechTagInstance {
 public:
  PartOfSpeechTagInstance();
  ~PartOfSpeechTagInstance();

  static const int kPOSTagS = 0;
  static const int kOutOfVocabularyI = 0;

  const char *part_of_speech_tag_at(int position) const {
    return instance_data_->string_at(position, kPOSTagS);
  }

  // return true if it is a out-of-vocabulary word or it doesnt't have tag
  // information in HMM data file
  bool is_out_of_vocabulary_word_at(int position) const {
    return instance_data_->integer_at(position, kOutOfVocabularyI) != 0;
  }

  // Set the size of this instance
  void set_size(int size) { instance_data_->set_size(size); }

  // Get the size of this instance
  int size() const { return instance_data_->size(); }

  // Set the value at position
  void set_value_at(int position, const char *tag, bool is_oov = true) {
    instance_data_->set_string_at(position, kPOSTagS, tag);
    instance_data_->set_integer_at(position, kOutOfVocabularyI, is_oov);
  }

 private:
  InstanceData *instance_data_;

  DISALLOW_COPY_AND_ASSIGN(PartOfSpeechTagInstance);
};

}  // namespace milkcat

#endif  // SRC_PARSER_PART_OF_SPEECH_TAG_INSTANCE_H_
