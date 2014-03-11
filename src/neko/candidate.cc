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
// candidate.cc --- Created at 2014-02-02
//

#include "neko/candidate.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "milkcat/libmilkcat.h"
#include "utils/readable_file.h"
#include "neko/maxent_classifier.h"
#include "neko/crf_vocab.h"
#include "neko/utf8.h"

namespace milkcat {

// Extracts features from name_str for the maxent classifier
std::vector<std::string> ExtractNameFeature(const char *name_str) {
  std::vector<std::string> feature_list;
  std::vector<std::string> utf8_chars;

  // Split the name_str into a vector of utf8 characters
  const char *it = name_str;
  char utf8_char[16], *end;
  while (*it) {
    uint32_t cp = utf8::next(it);
    end = utf8::append(cp, utf8_char);
    *end = '\0';
    utf8_chars.push_back(utf8_char);
  }

  if (utf8_chars.size() >= 2) {
    feature_list.push_back(std::string("B:") + utf8_chars[0]);
    feature_list.push_back(std::string("E:") +
                           utf8_chars[utf8_chars.size() - 1]);
    for (auto it = utf8_chars.begin() + 1; it < utf8_chars.end() - 1; ++it) {
      feature_list.push_back(std::string("M:") + *it);
    }
  }

  return feature_list;
}

// Get the candidate from crf segmentation vocabulary specified by crf_vocab.
// Returns a map the key is the word, and the value is its cost in unigram.
std::unordered_map<std::string, float> GetCandidate(
    const char *model_path,
    const char *vocabulary_path,
    const std::unordered_map<std::string, int> &crf_vocab,
    int total_count,
    int (* log_func)(const char *message),
    Status *status,
    int thres_freq) {
  std::unordered_map<std::string, float> candidates;
  char msg_text[1024];

  ModelFactory *model_factory = new ModelFactory(MODEL_PATH);

  // If vocabulary_path != nullptr use vocabulary_path as OOV-filter otherwise
  // use system dictionary as the OOV-word filter 
  const TrieTree *index = nullptr;
  if (vocabulary_path) {
    model_factory->SetUserDictionary(vocabulary_path);
    index = model_factory->UserIndex(status);
  } else {
    index = model_factory->Index(status);
  }

  MaxentModel *name_model = nullptr;
  if (status->ok()) name_model = MaxentModel::New(model_path, status);

  MaxentClassifier *classifier = nullptr;
  if (status->ok()) classifier = new MaxentClassifier(name_model);

  // Calculate the thres_freq via total_count if thres_freq = kDefaultThresFreq
  if (thres_freq == kDefaultThresFreq)
    thres_freq = static_cast<int>(atan(1e-7 * total_count) * 50) + 2;

  if (status->ok() && log != nullptr) {
    snprintf(msg_text,
             sizeof(msg_text),
             "Threshold frequency of candidates is %d",
             thres_freq);
    log_func(msg_text);
  }

  int person_name = 0;
  std::vector<std::string> feature;
  if (status->ok()) {
    for (auto &x : crf_vocab) {
      // If the word frequency is greater than the threshold value and it not
      // exists in the original vocabulary
      if (x.second > thres_freq && index->Search(x.first.c_str()) < 0) {
        feature = ExtractNameFeature(x.first.c_str());

        // Filter one character word
        if (feature.size() == 0) continue;
        const char *y = classifier->Classify(feature);

        // And if it is not a name
        if (strcmp(y, "F") == 0) {
          candidates[x.first] = -log(
              static_cast<float>(x.second) / total_count);
        } else {
          person_name++;
        }
      }
    }
  }

  if (status->ok() && log != nullptr) {
    snprintf(msg_text,
             sizeof(msg_text),
             "Filtered %d person names.",
             person_name);
    log_func(msg_text);
  }

  delete name_model;
  delete classifier;
  delete model_factory;

  return candidates;
}

}  // namespace milkcat
