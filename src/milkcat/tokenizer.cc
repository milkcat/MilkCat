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
// tokenization.cc
// tokenizer.cc --- Created at 2013-12-24
//

#include "milkcat/tokenizer.h"

#include <stdio.h>
#include "common/milkcat_config.h"
#include "milkcat/token_instance.h"
#include "milkcat/token_lex.h"

namespace milkcat {

Tokenization::Tokenization(): buffer_alloced_(false) {
  milkcat_yylex_init(&yyscanner);
}

Tokenization::~Tokenization() {
  if (buffer_alloced_ == true) {
    milkcat_yy_delete_buffer(yy_buffer_state_, yyscanner);
  }

  milkcat_yylex_destroy(yyscanner);
}

void Tokenization::Scan(const char *buffer_string) {
  if (buffer_alloced_ == true) {
    milkcat_yy_delete_buffer(yy_buffer_state_, yyscanner);
  }
  buffer_alloced_ = true;
  yy_buffer_state_ = milkcat_yy_scan_string(buffer_string, yyscanner);
}

bool Tokenization::GetSentence(TokenInstance *token_instance) {
  int token_type;
  int token_count = 0;

  while (token_count < kTokenMax - 1) {
    token_type = milkcat_yylex(yyscanner);

    if (token_type == TokenInstance::kEnd) break;

    token_instance->set_value_at(token_count,
                                 milkcat_yyget_text(yyscanner),
                                 token_type);
    token_count++;

    if (token_type == TokenInstance::kPeriod ||
        token_type == TokenInstance::kCrLf)
      break;
  }

  token_instance->set_size(token_count);
  return token_count != 0;
}

}  // namespace milkcat
