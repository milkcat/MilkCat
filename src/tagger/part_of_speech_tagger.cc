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
// part_of_speech_tagger.cc --- Created at 2014-11-11
//

#include "tagger/part_of_speech_tagger.h"

#include <string.h>
#include "include/milkcat.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "util/readable_file.h"
#include "util/status.h"
#include "util/util.h"

namespace milkcat {

void PartOfSpeechTagger::ReadInstance(
    ReadableFile *fd,
    TermInstance *term_instance,
    PartOfSpeechTagInstance *tag_instance,
    Status *status) {
  char line[16384];
  char word[1024], tag[1024];
  int size = 0;

  if (status->ok()) fd->ReadLine(line, sizeof(line), status);
  if (status->ok()) {
    char *saveptr, *tok;
    trim(line);
    size = 0;
    tok = strtok_r(line, " ", &saveptr);
    while (status->ok()) {
      if (tok == NULL) break;
      char *splitter = strrchr(tok, '_');
      if (splitter != NULL) {
        strlcpy(word, tok, splitter - tok + 1);
        strcpy(tag, splitter + 1);
        if (kTokenMax > size) {
          term_instance->set_value_at(size, word, 0, Parser::kChineseWord);
          tag_instance->set_value_at(size, tag);
          size++;            
        }
      }
      tok = strtok_r(NULL, " ", &saveptr);
    }
    if (status->ok()) {
      term_instance->set_size(size);
      tag_instance->set_size(size);
    }
  }
}

double PartOfSpeechTagger::Test(const char *test_corpus,
                                PartOfSpeechTagger *tagger,
                                Status *status) {
  char *line = new char[1024 * 1024];
  ReadableFile *fd = ReadableFile::New(test_corpus, status);
  TermInstance *term_instance = new TermInstance();
  PartOfSpeechTagInstance *gold_tag_instance = new PartOfSpeechTagInstance();
  PartOfSpeechTagInstance *tag_instance = new PartOfSpeechTagInstance();

  int total = 0, correct = 0;

  while (status->ok() && !fd->Eof()) {
    ReadInstance(fd, term_instance, gold_tag_instance, status);
    if (status->ok()) {
      tagger->Tag(tag_instance, term_instance);
      for (int i = 0; i < tag_instance->size(); ++i) {
        ++total;
        /*
        printf("%s\t%s\t%s\n",
               term_instance->term_text_at(i),
               gold_tag_instance->part_of_speech_tag_at(i),
               tag_instance->part_of_speech_tag_at(i));
        */
        if (strcmp(tag_instance->part_of_speech_tag_at(i),
                   gold_tag_instance->part_of_speech_tag_at(i)) == 0) {
          ++correct;
        }
      }
    }
  }

  delete fd;
  delete[] line;
  delete term_instance;
  delete gold_tag_instance;
  delete tag_instance;
  
  if (status->ok() && total != 0) {
    return static_cast<double>(correct) / total;
  } else {
    return 0.0;
  } 
}

}  // namespace milkcat
