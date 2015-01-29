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
// dependency_parser_features.cc --- Created at 2014-06-03
// dependency_feature.cc --- Created at 2014-09-15
// feature_template.cc --- Created at 2014-10-27
//

#include "parser/feature_template.h"
#include "parser/feature_template-inl.h"

#include <assert.h>
#include <stdio.h>
#include <map>
#include "common/reimu_trie.h"
#include "ml/feature_set.h"
#include "parser/node.h"
#include "parser/state.h"
#include "segmenter/term_instance.h"
#include "tagger/part_of_speech_tag_instance.h"
#include "util/readable_file.h"
#include "util/string_builder.h"

namespace milkcat {

const char *DependencyParser::FeatureTemplate::kRootTerm = "ROOT";
const char *DependencyParser::FeatureTemplate::kRootTag = "ROOT";

DependencyParser::FeatureTemplate::FeatureTemplate(): 
    term_instance_(NULL),
    part_of_speech_tag_instance_(NULL),
    state_(NULL) {
}

DependencyParser::FeatureTemplate *
DependencyParser::FeatureTemplate::Open(const char *filename, Status *status) {
  // Read template file
  char line[1024];
  std::vector<std::string> feature_template;
  ReadableFile *fd = ReadableFile::New(filename, status);

  FeatureTemplate *self = new FeatureTemplate();

  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      trim(line);
      feature_template.push_back(line);
    }
  }
  delete fd;
  fd = NULL;

  // Compiles the template
  if (status->ok())  self->CompileTemplate(feature_template, status);

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

DependencyParser::FeatureTemplate::~FeatureTemplate() {
}


void DependencyParser::FeatureTemplate::CompileTemplate(
    const std::vector<std::string> &feature_template,
    Status *status) {
  ReimuTrie *feature_index = new ReimuTrie();
  feature_index->Put("STw", kSTw);
  feature_index->Put("STt", kSTt);
  feature_index->Put("N0w", kN0w);
  feature_index->Put("N0t", kN0t);
  feature_index->Put("N1w", kN1w);
  feature_index->Put("N1t", kN1t);
  feature_index->Put("N2t", kN2t);
  feature_index->Put("STPt", kSTPt);
  feature_index->Put("STLCt", kSTLCt);
  feature_index->Put("STRCt", kSTRCt);
  feature_index->Put("N0LCt", kN0LCt);
  feature_index->Put("N0RCt", kN0RCt);
  assert(kAtomicFeatureNumber < 127 - ' ');

  char feature_name[24];
  compiled_template_.clear();
  for (std::vector<std::string>::const_iterator it = feature_template.begin();
       it != feature_template.end() && status->ok();
       ++it) {
    std::string compiled_template;
    const char *templ = it->c_str();
    const char *p = templ, *q = NULL;
    while (*p && status->ok()) {
      if (*p != '[') {
        compiled_template += *p;
        p++;
      } else {
        const char *q = p;
        while (*q != ']') {
          if (*q == '\0') {
            *status = Status::Corruption(
                "Template file corrputed, missing ']'");
          }
          ++q;
        }
        int len = q - p - 1;
        strlcpy(feature_name, p + 1, len + 1);
        int fid = feature_index->Get(feature_name, -1);

        if (fid < 0) {
          std::string errmsg = "Template file corrputed, unexpected feature: ";
          errmsg += feature_name;
          *status = Status::Corruption(errmsg.c_str());
        }
        compiled_template += '[';
        compiled_template += static_cast<char>(' ' + fid);
        p = q + 1;
      }
    }
    compiled_template_.push_back(compiled_template);
  }

  delete feature_index;
}

int DependencyParser::FeatureTemplate::Extract(
    const State *state,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance,
    FeatureSet *feature_set) {
  Node *node;
  int feature_num = 0;

  state_ = state;
  term_instance_ = term_instance;
  part_of_speech_tag_instance_ = part_of_speech_tag_instance;

  // First initialize each feature string into atomic_feature_
  strlcpy(atomic_feature_[kSTw], STw(), kFeatureStringMax);
  strlcpy(atomic_feature_[kSTt], STt(), kFeatureStringMax);
  strlcpy(atomic_feature_[kN0w], N0w(), kFeatureStringMax);
  strlcpy(atomic_feature_[kN0t], N0t(), kFeatureStringMax);
  strlcpy(atomic_feature_[kN1w], N1w(), kFeatureStringMax);
  strlcpy(atomic_feature_[kN1t], N1t(), kFeatureStringMax);
  strlcpy(atomic_feature_[kN2t], N2t(), kFeatureStringMax);
  strlcpy(atomic_feature_[kSTPt], STPt(), kFeatureStringMax);
  strlcpy(atomic_feature_[kSTLCt], STLCt(), kFeatureStringMax);
  strlcpy(atomic_feature_[kSTRCt], STRCt(), kFeatureStringMax);
  strlcpy(atomic_feature_[kN0LCt], N0LCt(), kFeatureStringMax);
  strlcpy(atomic_feature_[kN0RCt], N0RCt(), kFeatureStringMax);

  feature_set->Clear();
  char feature_name[24];
  for (std::vector<std::string>::const_iterator 
       it = compiled_template_.begin();
       it != compiled_template_.end();
       ++it) {
    StringBuilder builder(feature_set->at(feature_num),
                          FeatureSet::kFeatureSizeMax);
    const char *templ = it->c_str();
    const char *p = templ, *q = NULL;
    while (*p) {
      if (*p != '[') {
        builder << *p;
        p++;
      } else {
        p++;
        int fid = *p - ' ';
        builder << atomic_feature_[fid];
        p++;
      }
    }
    feature_num++;
  }
  feature_set->set_size(feature_num);

  return feature_num;
}

}  // namespace milkcat