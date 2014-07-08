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
#include "utils/readable_file.h"

using milkcat::Parser;
using milkcat::Model;
using milkcat::Keyphrase;
using milkcat::Status;
using milkcat::ReadableFile;
using milkcat::Newword;

struct Options {
  int method;
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

Options::Options(): method(Parser::kDefault),
                    display_tag(true),
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
  printf("    milkcat keyphrase filename\n");
  printf("        Extracts keyphrases from filename.\n");
  printf("    milkcat newword filename\n");
  printf("        Extracts newwords from filename.\n");
  printf("Parser Options:\n");
  printf("    -d <path>    Set the path of model directory.\n");
  printf("    -u <path>    Set the path of user dictionary file.\n");
  printf("    -m <method>  Set the parsing method. methods are:\n");
  printf("        crf_seg     - Use CRF segmenter.\n");
  printf("        unigram_seg - Use Unigram segmenter.\n");
  printf("        unigram_seg - Use Bigram segmenter.\n");
  printf("        crf         - Use CRF segmenter and Part-Of-Speech tagger.\n");
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
          options->method = Parser::kCrfSegmenter | Parser::kNoTagger;
          options->display_tag = false;
        } else if (strcmp(optarg, "unigram_seg") == 0) {
          options->method = Parser::kUnigramSegmenter | Parser::kNoTagger;
          options->display_tag = false;
        } else if (strcmp(optarg, "crf") == 0) {
          options->method = Parser::kCrfSegmenter | Parser::kCrfTagger;
          options->display_tag = true;   
        } else if (strcmp(optarg, "mixed_seg") == 0) {
          options->method = Parser::kMixedSegmenter | Parser::kNoTagger;
          options->display_tag = false;   
        } else if (strcmp(optarg, "mixed") == 0) {
          options->method = Parser::kMixedSegmenter | Parser::kMixedTagger;
          options->display_tag = true;   
        } else if (strcmp(optarg, "bigram_seg") == 0) {
          options->method = Parser::kBigramSegmenter | Parser::kNoTagger;
          options->display_tag = false;   
        } else if (strcmp(optarg, "dep") == 0) {
          options->method = Parser::kParser;
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
  
  Parser *parser = Parser::New(model, options.method);

  if (parser == NULL) {
    fputs(milkcat::LastError(), stderr);
    fputs("\n", stderr);
    exit(1);
  }

  while (NULL != fgets(input_buffer, 1048576, fd)) {
    Parser::Iterator *it = parser->Parse(input_buffer);
    index = 0;
    while (!it->End()) {
      index++;
      switch (*it->word()) {
        case '\r':
        case '\n':
        case ' ':
          // Skip to next word
          it->Next();
          continue;
      }

      fputs(it->word(), stdout);

      if (!options.conll_format) {
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
        // Use conll format to output the dependdency parsing result
        fputs("\t", stdout);
        sprintf(buffer, "%d", index);
        fputs(buffer, stdout);

        fputs("\t", stdout);
        fputs(it->part_of_speech_tag(), stdout);

        fputs("\t", stdout);
        sprintf(buffer, "%d", it->head_node());
        fputs(buffer, stdout);

        fputs("\t", stdout);
        fputs(it->dependency_type(), stdout);

        fputs("\n", stdout);
      }

      it->Next();    
    }

    fputs("\n", stdout);
    parser->Release(it);
  }

  delete parser;
  delete model;
  delete[] input_buffer;
  if (!options.use_stdin) fclose(fd);

  return 0;
}

int KeyphraseMain(int argc, char **argv) {
  Status status;

  if (argc != 3) {
    PrintUsage();
    return 1;
  }
 
  Model *model = NULL;
  if (status.ok()) {
    model = Model::New();
    if (model == NULL) status = Status::Info(milkcat::LastError());
  }

  Keyphrase *keyphrase = NULL;
  if (status.ok()) {
    keyphrase = Keyphrase::New(model);
    if (keyphrase == NULL) status = Status::Info(milkcat::LastError());
  }

  const char *filename = argv[argc - 1];

  ReadableFile *fd = NULL;
  if (status.ok()) fd = ReadableFile::New(filename, &status);

  char *text = NULL;
  if (status.ok()) {
    int text_size = fd->Size();
    text = new char[text_size + 1];

    fd->Read(text, text_size, &status);
    text[text_size] = '\0';
  }

  Keyphrase::Iterator *it = NULL;
  if (status.ok()) {
    it = keyphrase->Extract(text);
    while (!it->End()) {
      printf("%s %lf\n", it->phrase(), it->weight());
      it->Next();
    }
    keyphrase->Release(it);
  }

  delete[] text;
  delete fd;
  delete keyphrase;
  delete model;

  if (!status.ok()) {
    fprintf(stderr, "%s\n", status.what());
    return 1;
  } else {
    return 0;
  }
}


// Display current progress of newword extraction
void DisplayProgress(int64_t bytes_processed,
                     int64_t file_size,
                     int64_t bytes_per_second) {
  fprintf(stderr,
          "\rprogress %dMB/%dMB -- %2.1f%% %.3fMB/s",
          (int)(bytes_processed / (1024 * 1024)),
          (int)(file_size / (1024 * 1024)),
          100.0 * bytes_processed / file_size,
          bytes_per_second / (double)(1024 * 1024));
  if (bytes_processed == file_size) fputs("\n", stderr);
  fflush(stderr);
}

// Display current status of newword extraction
void DisplayLog(const char *str) {
  fprintf(stderr, "%s\n", str);
}

int NekonkeoMain(int argc, char **argv) {
  if (argc != 3) {
    PrintUsage();
    return 1;
  }

  Newword *nw = Newword::New();
  nw->set_log_function(DisplayLog);
  nw->set_progress_function(DisplayProgress);

  Newword::Iterator *it = nw->Extract(argv[2]);

  while (!it->End()) {
    printf("%s %lf\n", it->word(), it->weight());
    it->Next();
  }
  nw->Release(it);

  delete nw;
  return 0;
}


int main(int argc, char **argv) {
  if (argc == 1) {
    PrintUsage();
  } else if (strcmp(argv[1], "keyphrase") == 0) {
    return KeyphraseMain(argc, argv);
  } else if (strcmp(argv[1], "newword") == 0) {
    return NekonkeoMain(argc, argv);
  } else {
    return ParserMain(argc, argv);
  }
}