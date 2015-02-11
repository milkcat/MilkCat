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

typedef struct mc_model_t {
  milkcat::Model *model;
} mc_model_t;

typedef struct mc_parser_t {
  milkcat::Parser *parser;
} mc_parser_t;

typedef struct mc_parseriter_internal_t {
  milkcat::Parser::Iterator *iterator;
} mc_parseriter_internal_t;

mc_model_t *mc_model_new(const char *model_path) {
  milkcat::Model *model = milkcat::Model::New(model_path);
  if (model == NULL) return NULL;
  
  mc_model_t *model_wrapper = new mc_model_t;
  model_wrapper->model = model;
  return model_wrapper;
}

void mc_model_delete(mc_model_t *model) {
  if (model == NULL) return ;
  delete model->model;
  delete model;
}

void mc_parseropt_init(mc_parseropt_t *parseropt) {
  parseropt->segmenter = MC_MIXED_SEGMENTER;
  parseropt->postagger = MC_HMM_POSTAGGER;
  parseropt->postagger = MC_NO_DEPPARSER;
}

mc_parser_t *mc_parser_new(mc_parseropt_t *parseropt, mc_model_t *model) {
  milkcat::Parser::Options option;
  switch (parseropt->segmenter) {
    case MC_BIGRAM_SEGMENTER:
      option.UseBigramSegmenter();
      break;
    case MC_CRF_SEGMENTER:
      option.UseCrfSegmenter();
      break;
    case MC_MIXED_SEGMENTER:
      option.UseMixedSegmenter();
      break;
    default:
      strlcpy(milkcat::gLastErrorMessage,
              "Illegal segmenter type",
              sizeof(milkcat::gLastErrorMessage));
      return NULL;
  }

  switch (parseropt->postagger) {
    case MC_FASTCRF_POSTAGGER:
      option.UseCrfPOSTagger();
      break;
    case MC_HMM_POSTAGGER:
      option.UseHmmPOSTagger();
      break;
    case MC_NO_POSTAGGER:
      option.NoPOSTagger();
      break;
    default:
      strlcpy(milkcat::gLastErrorMessage,
              "Illegal part-of-speech tagger type",
              sizeof(milkcat::gLastErrorMessage));
      return NULL;
  }

  switch (parseropt->depparser) {
    case MC_NO_DEPPARSER:
      option.NoDependencyParser();
      break;
    case MC_YAMADA_DEPPARSER:
      option.UseYamadaParser();
      break;
    case MC_BEAMYAMADA_DEPPARSER:
      option.UseBeamYamadaParser();
      break;
    default:
      strlcpy(milkcat::gLastErrorMessage,
              "Illegal dependency parser type",
              sizeof(milkcat::gLastErrorMessage));
      return NULL;
  }

  milkcat::Parser *parser = milkcat::Parser::New(option, model->model);
  if (parser == NULL) return NULL;

  mc_parser_t *parser_wrapper = new mc_parser_t;
  parser_wrapper->parser = parser;
  return parser_wrapper;
}

void mc_parser_delete(mc_parser_t *parser) {
  if (parser == NULL) return ;
  delete parser->parser;
  delete parser;
}

mc_parseriter_t *mc_parseriter_new() {
  mc_parseriter_t *parseriter = new mc_parseriter_t;
  parseriter->word = "";
  parseriter->part_of_speech_tag = "";
  parseriter->it = new mc_parseriter_internal_t;
  parseriter->it->iterator = new milkcat::Parser::Iterator();
  return parseriter;
}

void mc_parseriter_delete(mc_parseriter_t *parseriter) {
  delete parseriter->it->iterator;
  delete parseriter->it;
  delete parseriter;
}

int mc_parseriter_end(mc_parseriter_t *parseriter) {
  return parseriter->it->iterator->End();
}

int mc_parseriter_isbos(mc_parseriter_t *parseriter) {
  return parseriter->it->iterator->is_begin_of_sentence();
}

void mc_parseriter_next(mc_parseriter_t *parseriter) {
  milkcat::Parser::Iterator *it = parseriter->it->iterator;
  it->Next();
  parseriter->word = it->word();
  parseriter->part_of_speech_tag = it->part_of_speech_tag();
  parseriter->head = it->head();
  parseriter->label = it->dependency_label();
}

void mc_parser_predict(mc_parser_t *parser,
                       mc_parseriter_t *parseriter,
                       const char *text) {
  milkcat::Parser::Iterator *it = parseriter->it->iterator;
  parser->parser->Predict(it, text);
  parseriter->word = it->word();
  parseriter->part_of_speech_tag = it->part_of_speech_tag();
  parseriter->head = it->head();
  parseriter->label = it->dependency_label();
}

const char *mc_last_error() {
  return milkcat::LastError();
}