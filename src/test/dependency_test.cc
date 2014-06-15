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
// dependency_test.cc --- Created at 2014-06-15
//

#include "common/model_factory.h"
#include "include/milkcat.h"
#include "milkcat/dependency_instance.h"
#include "milkcat/dependency_parser.h"
#include "milkcat/term_instance.h"
#include "milkcat/part_of_speech_tag_instance.h"
#include "utils/log.h"
#include "utils/readable_file.h"
#include "utils/utils.h"

using milkcat::Status;
using milkcat::TermInstance;
using milkcat::PartOfSpeechTagInstance;
using milkcat::DependencyInstance;
using milkcat::DependencyParser;
using milkcat::Model;
using milkcat::ReadableFile;

void LoadSentence(ReadableFile *fd,
                  TermInstance *term_instance,
                  PartOfSpeechTagInstance *tag_instance,
                  DependencyInstance *dependency_instance,
                  Status *status) {
  char buf[1024], word[1024], tag[1024], type[1024];
  int term_num = 0, head = 0;
  while (status->ok()) {
    fd->ReadLine(buf, sizeof(buf), status);
    if (status->ok()) {
      // NULL line indicates the end of a sentence
      milkcat::utils::trim(buf);
      if (*buf == '\0') break;

      sscanf(buf, "%s %s %d %s", word, tag, &head, type);
      term_instance->set_value_at(term_num, word, 0, 0);
      tag_instance->set_value_at(term_num, tag);
      dependency_instance->set_value_at(term_num, type, head);
      term_num++;
    }
  }

  if (status->ok()) {
    term_instance->set_size(term_num);
    tag_instance->set_size(term_num);
    dependency_instance->set_size(term_num);
  }
}

int TestDependency(const char *gold_filename) {
  Status status;

  ReadableFile *fd = ReadableFile::New(gold_filename, &status);
  if (!status.ok()) return 77;

  Model::Impl *model_impl = new Model::Impl(MODEL_DIR);
  DependencyParser *parser = DependencyParser::New(model_impl, &status);
  ASSERT(status.ok(), status.what());

  int total = 0;
  int las_count = 0;
  int uas_count = 0;

  TermInstance *term_instance = new TermInstance();
  PartOfSpeechTagInstance *tag_instance = new PartOfSpeechTagInstance();
  DependencyInstance *dependency_instance_gold = new DependencyInstance();
  DependencyInstance *dependency_instance = new DependencyInstance();

  while (!fd->Eof()) {
    LoadSentence(fd,
                 term_instance,
                 tag_instance,
                 dependency_instance_gold,
                 &status);

    ASSERT(status.ok(), status.what());
    parser->Parse(dependency_instance, tag_instance, term_instance);

    for (int i = 0; i < term_instance->size(); ++i) {
      total++;
      if (dependency_instance_gold->head_node_at(i) == 
          dependency_instance->head_node_at(i)) {
        uas_count++;
        if (strcmp(dependency_instance_gold->dependency_type_at(i),
                   dependency_instance->dependency_type_at(i)) == 0) {
          las_count++;
        }
      }
    }
  }

  printf("LAS: %.3lf\n", static_cast<double>(las_count) / total);
  printf("UAS: %.3lf\n", static_cast<double>(uas_count) / total);

  delete term_instance;
  delete tag_instance;
  delete dependency_instance_gold;
  delete dependency_instance;
  delete parser;
  delete model_impl;

  return 0;
}

int main() {
  return TestDependency(PRIVATE_DIR "ctb7-test.malt");
}



