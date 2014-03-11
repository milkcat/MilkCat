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
// tokenization.h
// tokenizer.h --- Created at 2013-12-24
//

#ifndef SRC_MILKCAT_TOKENIZER_H_
#define SRC_MILKCAT_TOKENIZER_H_

#include "milkcat/token_lex.h"

namespace milkcat {

class TokenInstance;

class Tokenization {
 public:
  Tokenization();
  ~Tokenization();

  // Scan an string to get tokens
  void Scan(const char *buffer_string);

  bool GetSentence(TokenInstance *token_instance);

 private:
  YY_BUFFER_STATE yy_buffer_state_;
  yyscan_t yyscanner;
  bool buffer_alloced_;
};

}  // namespace milkcat

#endif  // SRC_MILKCAT_TOKENIZER_H_
