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
// libmilkcat_capi.cc --- Created at 2014-12-02
//

#include "include/milkcat.h"
#include "libmilkcat.h"

#include "util/util.h"

typedef struct milkcat_model_t {
  milkcat::Model *model;
} milkcat_model_t;

typedef struct milkcat_parser_t {
  milkcat::Parser *parser;
} milkcat_parser_t;

typedef struct milkcat_parseriter_internal_t {
  milkcat::Parser::Iterator *iterator;
} milkcat_parseriter_internal_t;

void milkcat_parseroptions_init(milkcat_parseroptions_t *parseropt) {
  parseropt->word_segmenter = MC_SEGMENTER_MIXED;
  parseropt->part_of_speech_tagger = MC_POSTAGGER_HMM;
  parseropt->dependency_parser = MC_DEPPARSER_NONE;
}

milkcat_parser_t *milkcat_parser_new(
    milkcat_parseroptions_t *parseropt) {
  milkcat::Parser::Options option;
  switch (parseropt->word_segmenter) {
    case MC_SEGMENTER_BIGRAM:
      option.UseBigramSegmenter();
      break;
    case MC_SEGMENTER_CRF:
      option.UseCRFSegmenter();
      break;
    case MC_SEGMENTER_MIXED:
      option.UseMixedSegmenter();
      break;
    default:
      milkcat::strlcpy(milkcat::gLastErrorMessage,
                       "Invalid type of segmenter",
                       sizeof(milkcat::gLastErrorMessage));
      return NULL;
  }

  switch (parseropt->part_of_speech_tagger) {
    case MC_POSTAGGER_MIXED:
      option.UseMixedPOSTagger();
      break;
    case MC_POSTAGGER_CRF:
      option.UseCRFPOSTagger();
      break;
    case MC_POSTAGGER_HMM:
      option.UseHMMPOSTagger();
      break;
    case MC_POSTAGGER_NONE:
      option.NoPOSTagger();
      break;
    default:
      milkcat::strlcpy(milkcat::gLastErrorMessage,
                       "Invalid type of part-of-speech tagger",
                       sizeof(milkcat::gLastErrorMessage));
      return NULL;
  }

  switch (parseropt->dependency_parser) {
    case MC_DEPPARSER_NONE:
      option.NoDependencyParser();
      break;
    case MC_DEPPARSER_YAMADA:
      option.UseYamadaParser();
      break;
    case MC_DEPPARSER_BEAMYAMADA:
      option.UseBeamYamadaParser();
      break;
    default:
      milkcat::strlcpy(milkcat::gLastErrorMessage,
                       "Illegal type of dependency parser",
                       sizeof(milkcat::gLastErrorMessage));
      return NULL;
  }

  // Sets model path and user dictionary
  if (parseropt->model_path) {
    option.SetModelPath(parseropt->model_path);
  }
  if (parseropt->user_dictionary_path) {
    option.SetUserDictionary(parseropt->user_dictionary_path);
  }

  milkcat::Parser *parser = new milkcat::Parser(option);
  if (parser == NULL) return NULL;

  milkcat_parser_t *parser_wrapper = new milkcat_parser_t;
  parser_wrapper->parser = parser;
  return parser_wrapper;
}

void milkcat_parser_destroy(milkcat_parser_t *parser) {
  if (parser == NULL) return ;
  delete parser->parser;
  delete parser;
}

void milkcat_parser_predict(
    milkcat_parser_t *parser,
    milkcat_parseriterator_t *parseriter,
    const char *text) {
  milkcat::Parser::Iterator *it = parseriter->it->iterator;
  parser->parser->Predict(it, text);
  parseriter->word = "";
  parseriter->part_of_speech_tag = "";
  parseriter->head = 0;
  parseriter->dependency_label = "";
  parseriter->is_begin_of_sentence = false;
}

milkcat_parseriterator_t *milkcat_parseriterator_new() {
  milkcat_parseriterator_t *parseriter = new milkcat_parseriterator_t;
  parseriter->word = "";
  parseriter->part_of_speech_tag = "";
  parseriter->it = new milkcat_parseriter_internal_t;
  parseriter->it->iterator = new milkcat::Parser::Iterator();
  return parseriter;
}

void milkcat_parseriterator_destroy(milkcat_parseriterator_t *parseriter) {
  delete parseriter->it->iterator;
  delete parseriter->it;
  delete parseriter;
}


bool milkcat_parseriterator_next(milkcat_parseriterator_t *parseriter) {
  milkcat::Parser::Iterator *it = parseriter->it->iterator;
  bool has_next = it->Next();
  parseriter->word = it->word();
  parseriter->part_of_speech_tag = it->part_of_speech_tag();
  parseriter->head = it->head();
  parseriter->dependency_label = it->dependency_label();
  parseriter->is_begin_of_sentence = it->is_begin_of_sentence();
  return has_next;
}

const char *milkcat_last_error() {
  return milkcat::LastError();
}
