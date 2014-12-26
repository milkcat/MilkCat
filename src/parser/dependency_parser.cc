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
// dependency_parser.cc --- Created at 2014-10-27
//

#include "parser/dependency_parser.h"

#include <stdio.h>
#include <set>
#include <string>
#include "ml/feature_set.h"
#include "ml/averaged_multiclass_perceptron.h"
#include "ml/multiclass_perceptron.h"
#include "ml/multiclass_perceptron_model.h"
#include "parser/dependency_parser.h"
#include "parser/node.h"
#include "parser/orcale.h"
#include "parser/state.h"
#include "parser/tree_instance.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "utils/pool.h"
#include "utils/readable_file.h"
#include "utils/status.h"
#include "utils/utils.h"

namespace milkcat {

DependencyParser::DependencyParser(MulticlassPerceptronModel *perceptron_model,
                                   FeatureTemplate *feature) {
  node_pool_ = new Pool<Node>();
  feature_set_ = new FeatureSet();
  perceptron_ = new MulticlassPerceptron(perceptron_model);
  feature_ = feature;

  // Initialize the yid information from the prediction of perceptron
  yid_transition_.resize(perceptron_model->ysize());
  yid_label_.resize(perceptron_model->ysize());
  for (int yid = 0; yid < perceptron_model->ysize(); ++yid) {
    const char *yname = perceptron_model->yname(yid);
    if (strncmp(yname, "leftarc", 7) == 0) {
      yid_transition_[yid] = kLeftArc;
      yid_label_[yid] = yname + 8;
    } else if (strncmp(yname, "rightarc", 8) == 0) {
      yid_transition_[yid] = kRightArc;
      yid_label_[yid] = yname + 9;
    } else if (strcmp(yname, "shift") == 0) {
      yid_transition_[yid] = kShift;
    } else {
      std::string err = "Unexpected label: ";
      err += yname;
      ERROR(err.c_str());
    }
    
    if (strcmp(yname, "rightarc_ROOT") == 0 ||
        strcmp(yname, "rightarc_root") == 0) {
      rightrarc_root_yid_ = yid;
    }
  }
}

int DependencyParser::ysize() {
  return perceptron_->ysize();
}

void DependencyParser::StateMove(State *state, int yid) const {
  int transition = yid_transition_[yid];
  const char *label = yid_label_[yid].c_str();
  state->set_last_transition(yid);
  switch (transition) {
    case kLeftArc:
      state->LeftArc(label);
      break;
    case kRightArc:
      state->RightArc(label);
      break;
    case kShift:
      state->Shift();
      break;
    default:
      ERROR("Unexpected transition");
  }  
}

bool DependencyParser::Allow(const State *state, int yid) const {
  int transition = yid_transition_[yid];
  switch (transition) {
    case kLeftArc:
      return state->AllowLeftArc();
    case kRightArc:
      return state->AllowRightArc(rightrarc_root_yid_ == yid);
    case kShift:
      return state->AllowShift();
    default:
      ERROR("Unexpected transition");
      return true;
  }
}

void DependencyParser::StoreStateIntoInstance(
    State *state,
    TreeInstance *instance) const {
  for (int i = 0; i < state->sentence_length() - 1; ++i) {
    // ignore the ROOT node
    const Node *node = state->node_at(i + 1);
    instance->set_value_at(i,  node->dependency_label(), node->head_id());
  }
  instance->set_size(state->sentence_length() - 1);
}

DependencyParser::~DependencyParser() {
  delete perceptron_;
  perceptron_ = NULL;

  delete feature_set_;
  feature_set_ = NULL;

  delete node_pool_;
  node_pool_ = NULL;
}

void DependencyParser::LoadDependencyTreeInstance(
    ReadableFile *fd,
    TermInstance *term_instance,
    PartOfSpeechTagInstance *tag_instance,
    TreeInstance *tree_instance,
    Status *status) {
  char buf[1024], word[1024], tag[1024], type[1024];
  int term_num = 0, head = 0;
  while (!fd->Eof() && status->ok()) {
    fd->ReadLine(buf, sizeof(buf), status);
    if (status->ok()) {
      // NULL line indicates the end of a sentence
      trim(buf);
      if (*buf == '\0') break;

      sscanf(buf, "%s %s %d %s", word, tag, &head, type);
      term_instance->set_value_at(term_num, word, 0, 0);
      tag_instance->set_value_at(term_num, tag);
      tree_instance->set_value_at(term_num, type, head);
      term_num++;
    }
  }

  if (status->ok()) {
    term_instance->set_size(term_num);
    tag_instance->set_size(term_num);
    tree_instance->set_size(term_num);
  }
}

void DependencyParser::Test(
    const char *test_corpus,
    DependencyParser *parser,
    double *LAS,
    double *UAS,
    Status *status) {
  int total = 0;
  int las_count = 0;
  int uas_count = 0;

  TermInstance *term_instance = new TermInstance();
  PartOfSpeechTagInstance *tag_instance = new PartOfSpeechTagInstance();
  TreeInstance *tree_instance_gold = new TreeInstance();
  TreeInstance *tree_instance = new TreeInstance();

  ReadableFile *fd = ReadableFile::New(test_corpus, status);

  while (status->ok() && !fd->Eof()) {
    LoadDependencyTreeInstance(
        fd,
        term_instance,
        tag_instance,
        tree_instance_gold,
        status);
    if (!status->ok()) break;

    parser->Parse(tree_instance, term_instance, tag_instance);
    for (int i = 0; i < term_instance->size(); ++i) {      

      // Ignore punctions
      if (strcmp(tag_instance->part_of_speech_tag_at(i), "PU") == 0 ||
          tag_instance->part_of_speech_tag_at(i)[0] == 'w')
        continue;
      total++;
      if (tree_instance_gold->head_node_at(i) == 
          tree_instance->head_node_at(i)) {
        uas_count++;
        if (strcmp(tree_instance_gold->dependency_type_at(i),
                   tree_instance->dependency_type_at(i)) == 0) {
          las_count++;
        }
      }
    }
    // parser->PrintCorrectTranstion(tree_instance_gold);
  }

  if (status->ok()) {
    *UAS = static_cast<double>(uas_count) / total;
    *LAS = static_cast<double>(las_count) / total;
  }

  delete term_instance;
  delete tag_instance;
  delete tree_instance_gold;
  delete tree_instance;
  delete fd;
}

void DependencyParser::PrintCorrectTranstion(TreeInstance *instance) {
  Orcale *orcale = new Orcale();
  orcale->Parse(instance);
  const char *label = NULL;
  while ((label = orcale->Next()) != NULL) {
    printf("%s  ", label);
  }
  printf("\n");

  delete orcale;
}

}  // namespace milkcat