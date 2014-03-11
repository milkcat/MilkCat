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
// term_instance.h --- Created at 2013-10-20
//


#ifndef SRC_MILKCAT_TERM_INSTANCE_H_
#define SRC_MILKCAT_TERM_INSTANCE_H_

#include <stdio.h>
#include "utils/utils.h"
#include "milkcat/instance_data.h"
#include "milkcat/token_instance.h"

namespace milkcat {

class TagSequence;

class TermInstance {
 public:
  TermInstance();
  ~TermInstance();

  static const int kTermTextS = 0;
  static const int kTermTokenNumberI = 0;
  static const int kTermTypeI = 1;
  static const int kTermIdI = 2;

  static const int kTermIdNone = -2;
  static const int kTermIdOutOfVocabulary = -1;

  enum {
    kChineseWord = 0,
    kEnglishWord = 1,
    kNumber = 2,
    kSymbol = 3,
    kPunction = 4,
    kOther = 5
  };

  // Get the term's string value at position
  const char *term_text_at(int position) const {
    return instance_data_->string_at(position, kTermTextS);
  }

  // Get the term type at position
  int term_type_at(int position) const {
    return instance_data_->integer_at(position, kTermTypeI);
  }

  // Get the token number of this term at position
  int token_number_at(int position) const {
    return instance_data_->integer_at(position, kTermTokenNumberI);
  }

  // Get the id of term (in unigram index)
  int term_id_at(int position) const {
    return instance_data_->integer_at(position, kTermIdI);
  }

  // Set the term id at position
  void set_term_id_at(int position, int term_id) {
    instance_data_->set_integer_at(position, kTermIdI, term_id);
  }

  // Set the size of this instance
  void set_size(int size) { instance_data_->set_size(size); }

  // Get the size of this instance
  int size() const { return instance_data_->size(); }

  // Set the value at position
  void set_value_at(int position,
                    const char *term,
                    int token_number,
                    int term_type,
                    int term_id = kTermIdNone) {
    instance_data_->set_string_at(position, kTermTextS, term);
    instance_data_->set_integer_at(position, kTermTokenNumberI, token_number);
    instance_data_->set_integer_at(position, kTermTypeI, term_type);
    instance_data_->set_integer_at(position, kTermIdI, term_id);
  }

 private:
  InstanceData *instance_data_;

  DISALLOW_COPY_AND_ASSIGN(TermInstance);
};

inline int TokenTypeToTermType(int token_type) {
  switch (token_type) {
    case TokenInstance::kChineseChar:
      return TermInstance::kChineseWord;
    case TokenInstance::kSpace:
    case TokenInstance::kCrLf:
      return TermInstance::kOther;
    case TokenInstance::kPeriod:
    case TokenInstance::kPunctuation:
    case TokenInstance::kOther:
      return TermInstance::kPunction;
    case TokenInstance::kEnglishWord:
      return TermInstance::kEnglishWord;
    case TokenInstance::kSymbol:
      return TermInstance::kSymbol;
    case TokenInstance::kNumber:
      return TermInstance::kNumber;
    default:
      return TermInstance::kOther;
  }
}

}  // namespace milkcat

#endif  // SRC_MILKCAT_TERM_INSTANCE_H_
