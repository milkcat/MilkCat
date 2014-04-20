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
#include "include/milkcat.h"

int print_usage() {
  fprintf(stderr, "Usage: milkcat [-m hmm_crf|crf|crf_seg] [-d model_directory] [-i] [-t] [file]\n");
  return 0;
}

const char *word_type_str(int word_type) {
  switch (word_type) {
   case MC_CHINESE_WORD:
    return "ZH";
   case MC_ENGLISH_WORD:
    return "EN";
   case MC_NUMBER:
    return "NUM";
   case MC_SYMBOL:
    return "SYM";
   case MC_PUNCTION:
    return "PU";
   case MC_OTHER:
    return "OTH";
  }

  return NULL;
}

int main(int argc, char **argv) {

  char model_path[1024] = "";
  int method = DEFAULT_ANALYZER;
  char c;
  int errflag = 0;
  FILE *fp = NULL;
  int use_stdin_flag = 0;
  int display_tag = 1;
  int display_type = 0;
  char user_dict[1024] = "";

  while ((c = getopt(argc, argv, "iu:td:m:")) != -1) {
    switch (c) {
     case 'i':
      fp = stdin;
      use_stdin_flag = 1;
      break;

     case 'd':
      strcpy(model_path, optarg);
      if (model_path[strlen(model_path) - 1] != '/') 
        strcat(model_path, "/");
      break;

     case 'u':
      strcpy(user_dict, optarg);
      break;

     case 'm':
      if (strcmp(optarg, "crf_seg") == 0) {
        method = CRF_SEGMENTER;
        display_tag = 0;
      } else if (strcmp(optarg, "unigram_seg") == 0) {
        method = UNIGRAM_SEGMENTER;
        display_tag = 0;
      } else if (strcmp(optarg, "crf") == 0) {
        method = CRF_ANALYZER;
        display_tag = 1;      
      } else if (strcmp(optarg, "seg") == 0) {
        method = DEFAULT_SEGMENTER;
        display_tag = 0;
      } else if (strcmp(optarg, "hmm_crf") == 0) {
        method = DEFAULT_ANALYZER;
        display_tag = 1;
      } else if (strcmp(optarg, "bigram_seg") == 0) {
        method = BIGRAM_SEGMENTER;
        display_tag = 0;
      } else {
        errflag++;
      }
      break;

     case 't':
      display_type = 1;
      break;

     case ':':
      fprintf(stderr, "Option -%c requires an operand\n", optopt);
      errflag++;
      break;

     case '?':
      fprintf(stderr, "Unrecognized option: -%c\n", optopt);
      errflag++;
      break;
    }
  }

  if ((use_stdin_flag == 1 && argc - optind != 0) || (use_stdin_flag == 0 && argc - optind != 1)) {
    errflag++;
  }

  if (errflag) {
    print_usage();
    exit(2);
  }
  
  if (use_stdin_flag == 0) {
    fp = fopen(argv[optind], "r");
    if (fp == NULL) {
      fprintf(stderr, "Unable to open file: %s\n", argv[optind]);
      exit(1);
    }
  }

  char *input_buffer = (char *)malloc(1048576);
  milkcat_model_t *model = milkcat_model_new(*model_path == '\0'? NULL: model_path);
  milkcat_cursor_t *cursor = milkcat_cursor_new();

  if (*user_dict) {
    milkcat_model_set_userdict(model, user_dict);
  }
  
  milkcat_t *m = milkcat_new(model, method);

  if (m == NULL) {
    fputs(milkcat_last_error(), stderr);
    fputs("\n", stderr);
    return 1;
  }

  size_t sentence_length;
  int i;
  char ch;

  milkcat_item_t item;
  while (NULL != fgets(input_buffer, 1048576, fp)) {
    milkcat_analyze(m, cursor, input_buffer);
    while (MC_OK == milkcat_cursor_get_next(cursor, &item)) {
      // printf("22222222\n");
      switch (item.word[0]) {
       case '\r':
       case '\n':
       case ' ':
        continue;
      }

      fputs(item.word, stdout);

      if (display_type == 1) {
        fputs("_", stdout);
        fputs(word_type_str(item.word_type), stdout);
      }

      if (display_tag == 1) {
        fputs("/", stdout);
        fputs(item.part_of_speech_tag, stdout);
      }

      fputs("  ", stdout);
    }
    printf("\n");
  }

  milkcat_destroy(m);
  milkcat_cursor_destroy(cursor);
  milkcat_model_destroy(model);
  free(input_buffer);
  if (use_stdin_flag == 0)
    fclose(fp);
  return 0;
}
