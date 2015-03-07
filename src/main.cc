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
// main.c --- Created at 2013-09-03
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include "config.h"
#include "include/milkcat.h"
#include "util/readable_file.h"

using milkcat::Parser;
using milkcat::Model;
using milkcat::Status;
using milkcat::ReadableFile;

struct Options {
  Parser::Options parser_options;
  bool display_tag;
  bool display_type;
  bool use_stdin;
  bool has_userdict;
  bool use_default_model_dir;
  bool conll_format;
  std::string model_dir;
  std::string userdict_path;
  std::string filename;

  Options();
};

Options::Options(): display_tag(true),
                    display_type(false),
                    use_stdin(false),
                    has_userdict(false),
                    use_default_model_dir(true),
                    conll_format(false) {
}

int PrintUsage() {
  printf("MilkCat - natural language processing toolkit v%s\n", VERSION);
  printf("Usage:\n");
  printf("    milkcat [parser-options] filename|-i\n");
  printf("        Parses the text from filename or stdin (-i).\n");
  printf("Parser Options:\n");
  printf("    -d <path>    Set the path of model directory.\n");
  printf("    -u <path>    Set the path of user dictionary file.\n");
  printf("    -m <method>  Set the parsing method. methods are:\n");
  printf("        crf_seg     - Use CRF segmenter.\n");
  printf("        unigram_seg - Use Unigram segmenter.\n");
  printf("        unigram_seg - Use Bigram segmenter.\n");
  printf("        crf         - Use CRF segmenter and Part-Of-Speech tagger.\n");
  printf("        hmm         - Use CRF segmenter and HMM Part-Of-Speech tagger.\n");
  printf("        mixed       - Use Mixed CRF and HMM segmenter and Part-Of-Speech\n");
  printf("                      tagger (Default value).\n");
  printf("        mixed_seg   - Use Mixed CRF and HMM segmenter.\n");
  printf("        dep         - Use mixed segmenter and dependency parser.\n");
  printf("    -t           Display the type of word.\n");
  return 0;
}

const char *WordType(int word_type) {
  switch (word_type) {
    case Parser::kChineseWord:
      return "ZH";
    case Parser::kEnglishWord:
      return "EN";
    case Parser::kNumber:
      return "NUM";
    case Parser::kSymbol:
      return "SYM";
    case Parser::kPunction:
      return "PU";
    case Parser::kOther:
      return "OTH";
  }

  return NULL;
}

void GetArgs(int argc, char **argv, Options *options) {
  char c;
  char last_char;
  std::string model_dir;

  while ((c = getopt(argc, argv, "iu:td:m:")) != -1) {
    switch (c) {
      case 'i':
        options->use_stdin = true;
        break;

      case 'd':
        options->use_default_model_dir = false;
        model_dir = optarg;
        last_char = model_dir[model_dir.size() - 1];
        if (last_char != '/') model_dir.push_back('/');
        options->model_dir = model_dir;
        break;

      case 'u':
        options->userdict_path = optarg;
        options->has_userdict = true;
        break;

      case 'm':
        if (strcmp(optarg, "crf_seg") == 0) {
          options->parser_options.UseCRFSegmenter();
          options->parser_options.NoPOSTagger();
          options->display_tag = false;
        } else if (strcmp(optarg, "unigram_seg") == 0) {
          options->parser_options.UseUnigramSegmenter();
          options->parser_options.NoPOSTagger();
          options->display_tag = false;
        } else if (strcmp(optarg, "crf") == 0) {
          options->parser_options.UseCRFSegmenter();
          options->parser_options.UseCRFPOSTagger();
          options->display_tag = true;
        } else if (strcmp(optarg, "hmm") == 0) {
          options->parser_options.UseCRFSegmenter();
          options->parser_options.UseHMMPOSTagger();
          options->display_tag = true;   
        } else if (strcmp(optarg, "mixed_seg") == 0) {
          options->parser_options.UseMixedSegmenter();
          options->parser_options.NoPOSTagger();
          options->display_tag = false;   
        } else if (strcmp(optarg, "mixed") == 0) {
          options->parser_options.UseMixedSegmenter();
          options->parser_options.UseMixedPOSTagger();
          options->display_tag = true;   
        } else if (strcmp(optarg, "bigram_seg") == 0) {
          options->parser_options.UseBigramSegmenter();
          options->parser_options.NoPOSTagger();
          options->display_tag = false;   
        } else if (strcmp(optarg, "dep") == 0) {
          options->parser_options.UseYamadaParser();
          options->conll_format = true;
        } else if (strcmp(optarg, "beam_dep") == 0) {
          options->parser_options.UseBeamYamadaParser();
          options->conll_format = true;  
        } else {
          PrintUsage();
          exit(1);
        }
        break;

      case 't':
        options->display_type = true;
        break;

      case ':':
        fprintf(stderr, "Option -%c requires an operand\n", optopt);
        exit(1);
        break;

      case '?':
        fprintf(stderr, "Unrecognized option: -%c\n", optopt);
        exit(1);
        break;
    }
  }

  // If use stdin no filename should include
  if ((options->use_stdin && argc - optind != 0) ||
      (!options->use_stdin && argc - optind != 1)) {
    PrintUsage();
    exit(1);
  }

  if (!options->use_stdin) {
    options->filename = argv[optind];
  }
}


int ParserMain(int argc, char **argv) {
  FILE *fd = NULL;
  Options options;
  char buffer[1024];
  int index = 0;

  GetArgs(argc, argv, &options);
  
  if (!options.use_stdin) {
    fd = fopen(options.filename.c_str(), "r");
    if (fd == NULL) {
      fprintf(stderr, "Unable to open file: %s\n", options.filename.c_str());
      exit(1);
    }
  } else {
    fd = stdin;
  }

  char *input_buffer = new char[1048576];
  Model *model = NULL;
  if (options.use_default_model_dir) {
    model = Model::New();
  } else {
    model = Model::New(options.model_dir.c_str());
  }

  if (model == NULL) {
    fputs(milkcat::LastError(), stderr);
    fputs("\n", stderr);
    exit(1);
  }

  if (options.has_userdict) {
    if (false == model->SetUserDictionary(options.userdict_path.c_str())) {
      fprintf(stderr,
              "Unable to load user dictionary file %s\n",
              options.userdict_path.c_str());
      exit(1);
    }
  }
  
  Parser *parser = Parser::New(options.parser_options, model);
  Parser::Iterator *it = new Parser::Iterator();

  if (parser == NULL) {
    fputs(milkcat::LastError(), stderr);
    fputs("\n", stderr);
    exit(1);
  }

  while (NULL != fgets(input_buffer, 1048576, fd)) {
    parser->Predict(it, input_buffer);
    index = 0;
    while (it->Next()) {
      switch (*it->word()) {
        case '\r':
        case '\n':
        case ' ':
          continue;
      }

      if (!options.conll_format) {
        fputs(it->word(), stdout);
        if (options.display_type) {
          fputs("_", stdout);
          fputs(WordType(it->type()), stdout);
        }

        if (options.display_tag) {
          fputs("/", stdout);
          fputs(it->part_of_speech_tag(), stdout);
        }

        fputs("  ", stdout);
      
      } else {
        if (it->is_begin_of_sentence() && index != 0) {
          index = 1;
          fputs("\n", stdout);
        } else {
          ++index;
        }

        sprintf(buffer, "%d", index);
        fputs(buffer, stdout);

        fputs("\t", stdout);
        fputs(it->word(), stdout);

        fputs("\t", stdout);
        fputs(it->part_of_speech_tag(), stdout);

        fputs("\t", stdout);
        sprintf(buffer, "%d", it->head());
        fputs(buffer, stdout);

        fputs("\t", stdout);
        fputs(it->dependency_label(), stdout);

        fputs("\n", stdout);
      } 
    }

    fputs("\n", stdout);
  }

  delete parser;
  delete it;
  delete model;
  delete[] input_buffer;
  if (!options.use_stdin) fclose(fd);

  return 0;
}


int main(int argc, char **argv) {
  if (argc == 1) {
    PrintUsage();
    return 1;
  }
  return ParserMain(argc, argv);
}