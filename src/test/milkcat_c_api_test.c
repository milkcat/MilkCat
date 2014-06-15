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
// milkcat_c_api_test.c --- Created at 2014-05-19
//

// Test program for MilkCat C API

#include "include/milkcat-c.h"

#include <assert.h>
#include <string.h>

const char test_text[] = "这个是MilkCat的简单测试。";
const char correct_word[][16] = {
  "这个", "是", "MilkCat", "的", "简单", "测试", "。"
};
const char correct_tag[][16] = {
  "PN", "VC", "NN", "DEG", "JJ", "NN", "PU"
};
const int correct_type[] = {
  kChineseWord,
  kChineseWord,
  kEnglishWord,
  kChineseWord,
  kChineseWord,
  kChineseWord,
  kPunction
};
int correct_length = 7;

int test_parser() {
  milkcat_parser_t parser;
  milkcat_model_t model;
  milkcat_parser_iterator_t it;
  int i;

  assert(milkcat_model_create(&model, MODEL_DIR) == 0);
  assert(milkcat_parser_create(&parser, model, kDefault) == 0);

  it = milkcat_parser_parse(parser, test_text);
  for (i = 0; i < correct_length; ++i) {
    assert(milkcat_parser_iterator_has_next(it));
    assert(strcmp(milkcat_parser_iterator_word(it), correct_word[i]) == 0);
    assert(strcmp(milkcat_parser_iterator_tag(it), correct_tag[i]) == 0);
    assert(milkcat_parser_iterator_type(it) == correct_type[i]);
    milkcat_parser_iterator_next(it);
  }
  milkcat_parser_release_iterator(parser, it);
  milkcat_parser_destroy(parser);
  milkcat_model_destroy(model);

  return 0;
}

int main() {
  test_parser();

  return 0;
}
