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
// token_instance.h --- Created at 2013-10-20
//

#ifndef SRC_MILKCAT_TOKEN_INSTANCE_H_
#define SRC_MILKCAT_TOKEN_INSTANCE_H_

#include <assert.h>
#include "utils/utils.h"
#include "milkcat/milkcat_config.h"
#include "milkcat/instance_data.h"

namespace milkcat {

class TokenInstance {
 public:
  TokenInstance();
  ~TokenInstance();

  enum {
    kEnd = 0,
    kSpace = 1,
    kChineseChar = 2,
    kCrLf = 3,
    kPeriod = 4,
    kNumber = 5,
    kEnglishWord = 6,
    kPunctuation = 7,
    kSymbol = 8,
    kOther = 9
  };


  static const int kTokenTypeI = 0;
  static const int kTokenUTF8S = 0;

  // Get the token's string value at position
  const char *token_text_at(int position) const {
    return instance_data_->string_at(position, kTokenUTF8S);
  }

  // Get the token type at position
  int token_type_at(int position) const {
    return instance_data_->integer_at(position, kTokenTypeI);
  }

  // Set the size of this instance
  void set_size(int size) { instance_data_->set_size(size); }

  // Get the size of this instance
  int size() const { return instance_data_->size(); }

  // Set the value at position
  void set_value_at(int position, const char *token_text, int token_type) {
    instance_data_->set_string_at(position, kTokenUTF8S, token_text);
    instance_data_->set_integer_at(position, kTokenTypeI, token_type);
  }

 private:
  InstanceData *instance_data_;
  DISALLOW_COPY_AND_ASSIGN(TokenInstance);
};

}  // namespace milkcat

#endif  // SRC_MILKCAT_TOKEN_INSTANCE_H_
