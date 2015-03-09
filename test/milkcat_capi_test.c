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
// milkcat_capi_test.c --- Created at 2014-12-02
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "include/milkcat.h"

#define TOKEN_MAX 64

char word[][TOKEN_MAX] = {
   "我", "的", "猫", "喜欢", "喝", "牛奶", "。"
};
char postag[][TOKEN_MAX] = {
   "PN", "DEG", "NN", "VV", "VV", "NN", "PU"
};
const int kLength = sizeof(word) / sizeof(char[TOKEN_MAX]);

void check_result(milkcat_parseriterator_t *it,
                  char (*word)[TOKEN_MAX],
                  char (*postag)[TOKEN_MAX],
                  int length) {
  for (int i = 0; i < length; ++i) {
    assert(milkcat_parseriterator_next(it));
    assert(strcmp(it->word, word[i]) == 0);
    assert(strcmp(it->part_of_speech_tag, postag[i]) == 0);
  }
  assert(!milkcat_parseriterator_next(it));
  assert(!milkcat_parseriterator_next(it));
}

int main() {
  milkcat_parseroptions_t parseropt;
  milkcat_parseroptions_init(&parseropt);
  parseropt.part_of_speech_tagger = MC_POSTAGGER_CRF;
  parseropt.model_path = MODEL_DIR;

  milkcat_parser_t *parser = milkcat_parser_new(&parseropt);
  if (parser == NULL) {
    puts(milkcat_last_error());
    return 1;
  }

  milkcat_parseriterator_t *it = milkcat_parseriterator_new();
  for (int i = 0; i < 5; ++i) {
    milkcat_parser_predict(parser, it, "我的猫喜欢喝牛奶。");
    check_result(it, word, postag, kLength);
  }

  milkcat_parseriterator_destroy(it);
  milkcat_parser_destroy(parser);
  return 0;
}

