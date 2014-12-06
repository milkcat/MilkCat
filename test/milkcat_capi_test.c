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
#include <string.h>
#include "include/milkcat.h"

int main() {
  mc_model_t *model = mc_model_new(MODEL_DIR);
  assert(model);

  mc_parseropt_t parseropt;
  mc_parseropt_init(&parseropt);
  parseropt.postagger = MC_FASTCRF_POSTAGGER;

  mc_parser_t *parser = mc_parser_new(&parseropt, model);
  mc_parseriter_t *it = mc_parseriter_new();
  mc_parser_parse(parser, it, "今天的天气不错。");

  assert(mc_parseriter_end(it) == false);
  assert(strcmp(it->word, "今天") == 0);
  assert(strcmp(it->part_of_speech_tag, "NT") == 0);
  mc_parseriter_next(it);

  assert(mc_parseriter_end(it) == false);
  assert(strcmp(it->word, "的") == 0);
  assert(strcmp(it->part_of_speech_tag, "DEG") == 0);
  mc_parseriter_next(it);

  assert(mc_parseriter_end(it) == false);
  assert(strcmp(it->word, "天气") == 0);
  assert(strcmp(it->part_of_speech_tag, "NN") == 0);
  mc_parseriter_next(it);

  assert(mc_parseriter_end(it) == false);
  assert(strcmp(it->word, "不错") == 0);
  assert(strcmp(it->part_of_speech_tag, "VA") == 0);
  mc_parseriter_next(it);

  assert(mc_parseriter_end(it) == false);
  assert(strcmp(it->word, "。") == 0);
  assert(strcmp(it->part_of_speech_tag, "PU") == 0);
  mc_parseriter_next(it);

  assert(mc_parseriter_end(it) == true);

  mc_parseriter_delete(it);
  mc_parser_delete(parser);
  mc_model_delete(model);
  return 0;
}

