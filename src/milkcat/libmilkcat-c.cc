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
// libmilkcat-c.cc --- Created at 2014-05-19
//

// The implementation of MilkCat C APIs which defined in milkcat-c.h

#include "include/milkcat-c.h"

#include <stdlib.h>
#include "include/milkcat.h"

using milkcat::Model;
using milkcat::Parser;

int milkcat_parser_create(milkcat_parser_t *parser,
                          milkcat_model_t model,
                          int type) {
  Model *internal_model = reinterpret_cast<Model *>(model);
  Parser *internal_parser = Parser::New(internal_model, type);
  *parser = reinterpret_cast<milkcat_parser_t>(internal_parser);

  if (internal_parser) {
    return 0;
  } else {
    return 1;
  }
}

void milkcat_parser_destroy(milkcat_parser_t parser) {
  Parser *internal_parser = reinterpret_cast<Parser *>(parser);
  delete internal_parser;
}

milkcat_parser_iterator_t
milkcat_parser_parse(milkcat_parser_t parser, const char *text) {
  Parser *internal_parser = reinterpret_cast<Parser *>(parser);
  Parser::Iterator *it = internal_parser->Parse(text);

  return reinterpret_cast<milkcat_parser_iterator_t>(it);
}

void milkcat_parser_release_iterator(milkcat_parser_t parser,
                                     milkcat_parser_iterator_t it) {
  Parser *internal_parser = reinterpret_cast<Parser *>(parser);
  Parser::Iterator *iterator = reinterpret_cast<Parser::Iterator *>(it);
  internal_parser->Release(iterator);
}

const char *milkcat_parser_iterator_word(milkcat_parser_iterator_t it) {
  Parser::Iterator *iterator = reinterpret_cast<Parser::Iterator *>(it);
  return iterator->word();
}

const char *milkcat_parser_iterator_tag(milkcat_parser_iterator_t it) {
  Parser::Iterator *iterator = reinterpret_cast<Parser::Iterator *>(it);
  return iterator->part_of_speech_tag();  
}

int milkcat_parser_iterator_type(milkcat_parser_iterator_t it) {
  Parser::Iterator *iterator = reinterpret_cast<Parser::Iterator *>(it);
  return iterator->type();   
}

int milkcat_parser_iterator_end(milkcat_parser_iterator_t it) {
  Parser::Iterator *iterator = reinterpret_cast<Parser::Iterator *>(it);
  return iterator->End();  
}

void milkcat_parser_iterator_next(milkcat_parser_iterator_t it) {
  Parser::Iterator *iterator = reinterpret_cast<Parser::Iterator *>(it);
  iterator->Next();    
}

int milkcat_model_create(milkcat_model_t *model, const char *model_dir) {
  Model *internal_model = Model::New(model_dir);
  *model = reinterpret_cast<milkcat_model_t>(internal_model);

  if (internal_model) {
    return 0;
  } else {
    return 1;
  }
}

void milkcat_model_destroy(milkcat_model_t model) {
  Model *internal_model = reinterpret_cast<Model *>(model);
  delete internal_model;
}

const char *milkcat_last_error() {
  return milkcat::LastError();
}