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
// milkcat_api_test.cc --- Created at 2015-01-27
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "include/milkcat.h"
#include "util/encoding.h"

using milkcat::Model;
using milkcat::Parser;

const int kLength = 9;

const char *kSentence = "我的猫喜欢喝牛奶。猫。";
const char word[][64] = {
   "我", "的", "猫", "喜欢", "喝", "牛奶", "。", "猫", "。"
};
const char postag[][64] = {
   "PN", "DEG", "NN", "VV", "VV", "NN", "PU", "NN", "PU"
};
const int head[] = {
    3, 1, 4, 0, 4, 5, 4, 0, 1
};
const char label[][64] = {
   "NMOD", "DEG", "SBJ", "ROOT", "OBJ", "OBJ", "VMOD", "ROOT", "VMOD"
};
const char isbos[] = {
    true, false, false, false, false, false, false,
    true, false
};

const int kBigramTextLength = 9;
const char bigram_test_word[][64] = {
   "博丽灵梦", "是", "与", "雾雨魔理沙", "并列", "的", "第一", "自", "机"
};

void check_prediction(Parser::Iterator *parseriter, bool use_gbk) {
  milkcat::Encoding *encoding = new milkcat::Encoding();
  char gbk_word[256];

  for (int i = 0; i < kLength; ++i) {
    assert(parseriter->Next());
    assert(parseriter->is_begin_of_sentence() == isbos[i]);

    // Compares word (converts to GBK when needed)
    if (use_gbk) {
      encoding->UTF8ToGBK(word[i], gbk_word, sizeof(gbk_word));
      assert(strcmp(parseriter->word(), gbk_word) == 0);
    } else {
      puts(parseriter->word());
      assert(strcmp(parseriter->word(), word[i]) == 0);
    }
    
    assert(strcmp(parseriter->part_of_speech_tag(), postag[i]) == 0);
    assert(strcmp(parseriter->dependency_label(), label[i]) == 0);
    assert(parseriter->head() == head[i]);
  }
  assert(parseriter->Next() == false);

  delete encoding;
} 

int parser_test() {
  Model *model = Model::New(MODEL_DIR);
  assert(model);

  Parser::Options options;
  options.UseMixedSegmenter();
  options.UseMixedPOSTagger();
  options.UseBeamYamadaParser();

  // Testes beam yamada parser
  Parser *parser = Parser::New(options, model);
  assert(parser);
  Parser::Iterator *parseriter = new Parser::Iterator();
  parser->Predict(parseriter, kSentence);
  check_prediction(parseriter, false);
  delete parser;

  // Testes yamada parser
  options.UseYamadaParser();
  parser = Parser::New(options, model);
  assert(parser);
  parser->Predict(parseriter, kSentence);
  check_prediction(parseriter, false);
  delete parser;

  delete parseriter;
  delete model;
  return 0;
}

int empty_string_test() {
  Model *model = Model::New(MODEL_DIR);
  assert(model);

  Parser::Options options;
  Parser *parser = Parser::New(options, model);
  assert(parser);
  Parser::Iterator *parseriter = new Parser::Iterator();
  parser->Predict(parseriter, "");
  assert(parseriter->Next() == false);
  assert(parseriter->Next() == false);

  // Checks calling Next function after End() == true 
  parseriter->Next();
  parseriter->Next();
  delete parseriter;
  delete parser;
  delete model;
  
  return 0;
}

int bigram_segmenter_test() {
  // Prepares user dictionary
  FILE *fd = fopen("user.txt", "w");
  assert(fd);

  fputs("博丽灵梦\n", fd);
  fputs("雾雨魔理沙 2.0\n", fd);
  fputs("一自 100.0\n", fd);
  fputs("\n", fd);
  fclose(fd);

  Model *model = Model::New(MODEL_DIR);
  assert(model->SetUserDictionary("NOT_EXIST.txt") == false);
  assert(model->SetUserDictionary("user.txt") == true);
  assert(model);

  Parser::Options options;
  options.UseBigramSegmenter();
  options.NoPOSTagger();
  Parser *parser = Parser::New(options, model);
  assert(parser);
  Parser::Iterator *parseriter = new Parser::Iterator();
  parser->Predict(parseriter, "博丽灵梦是与雾雨魔理沙并列的第一自机");

  for (int i = 0; i < kBigramTextLength; ++i) {
    assert(parseriter->Next());
    assert(strcmp(parseriter->word(), bigram_test_word[i]) == 0);
  }
  assert(parseriter->Next() == false);

  delete parseriter;
  delete parser;
  delete model;
  
  return 0; 
}

int gbk_test() {
  Model *model = Model::New(MODEL_DIR);
  assert(model);

  Parser::Options options;
  options.UseMixedSegmenter();
  options.UseMixedPOSTagger();
  options.UseBeamYamadaParser();
  options.UseGBK();

  char gbk_sentence[2048];
  milkcat::Encoding *encoding = new milkcat::Encoding();
  encoding->UTF8ToGBK(kSentence, gbk_sentence, sizeof(gbk_sentence));
  delete encoding;

  Parser *parser = Parser::New(options, model);
  assert(parser);
  Parser::Iterator *parseriter = new Parser::Iterator();
  parser->Predict(parseriter, gbk_sentence);
  check_prediction(parseriter, true);
  delete parser;

  delete parseriter;
  delete model;
  return 0;
}

int main() {
  parser_test();
  empty_string_test();
  bigram_segmenter_test();
  gbk_test();

  return 0;
}
