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
// parser_train.cc --- Created at 2013-10-24
//

#define DEBUG

#include <stdio.h>
#include <set>
#include <string>
#include <vector>
#include "common/model_impl.h"
#include "include/milkcat.h"
#include "ml/feature_set.h"
#include "ml/multiclass_perceptron.h"
#include "ml/multiclass_perceptron_model.h"
#include "parser/dependency_instance.h"
#include "parser/dependency_parser.h"
#include "parser/naive_arceager_dependency_parser.h"
#include "parser/orcale.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/log.h"
#include "utils/readable_file.h"
#include "utils/utils.h"

using milkcat::Status;
using milkcat::TermInstance;
using milkcat::PartOfSpeechTagInstance;
using milkcat::DependencyInstance;
using milkcat::DependencyParser;
using milkcat::NaiveArceagerDependencyParser;
using milkcat::Model;
using milkcat::ReadableFile;

namespace milkcat {




void NaiveArcEagerParserTrain(const char *training_corpus,
                              const char *template_filename,
                              const char *model_prefix,
                              int max_iter,
                              Status *status) {
  // Some instances
  TermInstance *term_instance = new TermInstance();
  PartOfSpeechTagInstance *tag_instance = new PartOfSpeechTagInstance();
  DependencyInstance *dependency_instance_correct = new DependencyInstance();
  DependencyInstance *dependency_instance = new DependencyInstance();

  // Orcale to predict correct transitions
  Orcale *orcale = new Orcale();

  // Gets the label name of transitions
  ReadableFile *fd;
  std::set<std::string> yname_set;
  const char *label = NULL;
  if (status->ok()) fd = ReadableFile::New(training_corpus, status);
  while (status->ok() && !fd->Eof()) {
    LoadDependencyTreeInstance(
        fd,
        term_instance,
        tag_instance,
        dependency_instance,
        status);
    if (status->ok()) {
      orcale->Parse(dependency_instance);
      while ((label = orcale->Next()) != NULL) {
        yname_set.insert(label);
      }
    }
  }
  delete fd;
  fd = NULL;

  // Read template file
  char line[1024];
  std::vector<std::string> template_vector;
  if (status->ok()) fd = ReadableFile::New(template_filename, status);

  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      utils::trim(line);
      template_vector.push_back(line);
    }
  }
  delete fd;
  fd = NULL;

  // Creates perceptron model, percpetron and the parser
  NaiveArceagerDependencyParser *parser = NULL;
  MulticlassPerceptronModel *model = NULL;
  MulticlassPerceptron *percpetron = NULL;
  if (status->ok()) {
    std::vector<std::string> yname(yname_set.begin(), yname_set.end());
    model = new MulticlassPerceptronModel(yname);
    parser = new NaiveArceagerDependencyParser(model, template_vector);
    percpetron = new MulticlassPerceptron(model);
  }

  // Start training
  for (int iter = 0; iter < max_iter && status->ok(); ++iter) {
    const char *label;
    int yid, predict_yid;
    int correct = 0, total = 0;
    if (status->ok()) fd = ReadableFile::New(training_corpus, status);
    while (status->ok() && !fd->Eof()) {
      LoadDependencyTreeInstance(
          fd,
          term_instance,
          tag_instance,
          dependency_instance_correct,
          status);
      if (status->ok()) {
        orcale->Parse(dependency_instance_correct);
        parser->StartParse(term_instance, tag_instance);
        while ((label = orcale->Next()) != NULL) {
          yid = model->yid(label);
          ASSERT(yid != MulticlassPerceptronModel::kIdNone, "Unexpected label");
          predict_yid = parser->NextTransition();
          if (percpetron->Train(parser->feature_set(), label)) {
            ++correct;
          }
          /*
          const milkcat::FeatureSet *feature_set = parser->feature_set();
          printf("%s", label);
          for (int i = 0; i < feature_set->size(); ++i) {
            printf(" %s", feature_set->at(i));
          }
          puts("");

          if (yid == predict_yid) {
            ++correct;
          }
          */
          parser->Step(yid);
          ++total;
        }
        parser->StoreResult(dependency_instance, term_instance, tag_instance);
#ifdef DEBUG
        // Check whether the orcale is correct
        for (int i = 0; i < dependency_instance->size(); ++i) {
          ASSERT(dependency_instance->head_node_at(i) == 
                 dependency_instance_correct->head_node_at(i),
                 "Orcale failed");
        }
#endif
      }
    }
    delete fd;
    fd = NULL;
    
    printf("Iter %d: p = %.3f\n",
           iter + 1,
           static_cast<float>(correct) / total);
           
  }

  if (status->ok()) model->Save(model_prefix, status);
  if (!status->ok()) puts(status->what());

  delete term_instance;
  delete tag_instance;
  delete dependency_instance_correct;
  delete dependency_instance;
  delete orcale;
  delete model;
  delete parser;
  delete percpetron;
}

}  // namespace milkcat

int main(int argc, char **argv) {
  milkcat::Status status;
  milkcat::NaiveArcEagerParserTrain(argv[1], argv[2], argv[3], 10, &status);
  if (!status.ok()) puts(status.what());
  return 0;
}
