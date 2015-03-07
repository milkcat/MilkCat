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

int main() {
  milkcat_model_t *model = milkcat_model_new(MODEL_DIR);
  if (model == NULL) {
    puts(milkcat_last_error());
    return 1;
  }

  milkcat_parseropt_t parseropt;
  milkcat_parseropt_use_default(&parseropt);
  parseropt.part_of_speech_tagger = MC_POSTAGGER_CRF;

  milkcat_parser_t *parser = milkcat_parser_new(&parseropt, model);
  if (parser == NULL) {
    puts(milkcat_last_error());
    return 1;
  }

  milkcat_parseriter_t *it = milkcat_parseriter_new();
  for (int i = 0; i < 5000; ++i) {
    milkcat_parser_predict(parser, it, "今天的天气不错。");

    assert(milkcat_parseriter_next(it));
    assert(strcmp(it->word, "今天") == 0);
    assert(strcmp(it->part_of_speech_tag, "NT") == 0);

    assert(milkcat_parseriter_next(it));
    assert(strcmp(it->word, "的") == 0);
    assert(strcmp(it->part_of_speech_tag, "DEG") == 0);

    assert(milkcat_parseriter_next(it));
    assert(strcmp(it->word, "天气") == 0);
    assert(strcmp(it->part_of_speech_tag, "NN") == 0);

    assert(milkcat_parseriter_next(it));
    assert(strcmp(it->word, "不错") == 0);
    assert(strcmp(it->part_of_speech_tag, "VA") == 0);

    assert(milkcat_parseriter_next(it));
    assert(strcmp(it->word, "。") == 0);
    assert(strcmp(it->part_of_speech_tag, "PU") == 0);

    assert(!milkcat_parseriter_next(it));
    assert(!milkcat_parseriter_next(it));
  }

  milkcat_parseriter_destroy(it);
  milkcat_parser_destroy(parser);
  milkcat_model_destroy(model);
  return 0;
}

