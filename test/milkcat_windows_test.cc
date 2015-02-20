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
// milkcat_windows_test.cc --- Created at 2015-02-20
//

#include <milkcat.h>
#include <stdlib.h>
#include <stdio.h>

using milkcat::Parser;
using milkcat::Model;

void ErrorOccured() {
  puts(milkcat::LastError());
  exit(1);
}

int main() {
  Model *model = Model::New("data/");
  if (model == NULL) ErrorOccured();

  Parser *parser = Parser::New(Parser::Options(), model);
  if (parser == NULL) ErrorOccured();

  Parser::Iterator *it = new Parser::Iterator();
  parser->Predict(it, "ÎÒµÄÃ¨Ï²»¶ºÈÅ£ÄÌ¡£");

  while (!it->End()) {
    printf("%s/%s  ", it->word(), it->part_of_speech_tag());
    it->Next();
  }
  puts("");

  delete it;
  delete parser;
  return 0;
}