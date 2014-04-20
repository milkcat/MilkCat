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
// neko_main.cc --- Created at 2014-02-03
// libnekoneko.cc --- Created at 2014-03-20
//

#include "include/milkcat.h"
#include "nekoneko/libnekoneko.h"

#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include "utils/utils.h"
#include "utils/writable_file.h"

namespace milkcat {

const char *MUTUALINFO_DEBUG_FILE = "mi.txt";
const char *ADJENT_DEBUG_FILE = "adjent.txt";

bool ExtractNewWord(
    const char *corpus_path,
    const char *vocabulary_word_file,
    std::vector<std::pair<std::string, double> > *result,
    void (* log_func)(const char *msg),
    void (* progress_func)(int64_t bytes_processed,
                           int64_t file_size,
                           int64_t bytes_per_second)) {
  Status status;
  int total_count = 0;
  char logmsg[1024],
       vocabulary_file[1024] = "",
       output_file[1024] = "";
  int c;

  if (log_func) {
    snprintf(logmsg,
             sizeof(logmsg),
             "Segment corpus %s with CRF model.",
             corpus_path);
    log_func(logmsg);
  }

  utils::unordered_map<std::string, int> vocab;
  if (status.ok()) {
    GetCrfVocabulary(
        corpus_path,
        &total_count,
        &vocab,
        progress_func,
        &status);    
  }

  if (status.ok() && log_func) {
    snprintf(logmsg,
             sizeof(logmsg),
             "OK, %d words in corpus, vocabulary size is %d",
             total_count,
             static_cast<int>(vocab.size()));
    log_func(logmsg);
    log_func("Get candidates from vocabulary.");
  }

  utils::unordered_map<std::string, float> candidates;
  if (status.ok()) {
    std::string model_path = MODEL_PATH;
    model_path += "person_name.maxent";
    GetCandidate(
        model_path.c_str(),
        *vocabulary_file != '\0' ? vocabulary_file : NULL,
        vocab,
        total_count,
        &candidates,
        log_func,
        &status,
        kUseDefaultThresFreq);
  }

  if (status.ok() && log_func) {
    snprintf(logmsg,
             sizeof(logmsg),
             "Get %d candidates. Write to candidate_cost.txt",
             static_cast<int>(candidates.size()));
    log_func(logmsg);
  }

  WritableFile *fd = NULL;
  if (status.ok()) fd = WritableFile::New("candidate_cost.txt", &status);

  char line[1024];
  if (status.ok()) {
    for (utils::unordered_map<std::string, float>::iterator
         it = candidates.begin(); it != candidates.end(); ++it) {
      snprintf(line, sizeof(line), "%s %.5f", it->first.c_str(), it->second);
      fd->WriteLine(line, &status);
    }
  }

  delete fd;

  // Clear the vocab
  vocab = utils::unordered_map<std::string, int>();

  utils::unordered_map<std::string, double> adjacent_entropy;
  utils::unordered_map<std::string, double> mutual_information;

  if (status.ok() && log_func) {
    snprintf(logmsg,
             sizeof(logmsg),
             "Analyze %s with bigram segmentation.",
             corpus_path);
    log_func(logmsg);
  }

  if (status.ok()) {
    ExtractAdjacent(candidates,
                    corpus_path,
                    &adjacent_entropy,
                    &vocab,
                    progress_func,
                    &status);
  }

  if (status.ok()) {
    snprintf(logmsg,
             sizeof(logmsg),
             "Write adjacent entropy to %s",
             ADJENT_DEBUG_FILE);
    log_func(logmsg);
    WritableFile *wf = WritableFile::New(ADJENT_DEBUG_FILE, &status);
    for (utils::unordered_map<std::string, double>::iterator
         it = adjacent_entropy.begin(); it != adjacent_entropy.end(); ++it) {
      if (!status.ok()) break;

      snprintf(line, sizeof(line), "%s %.3f", it->first.c_str(), it->second);
      wf->WriteLine(line, &status);
    }
    delete wf;
  }

  if (status.ok()) {
    log_func("Calculate mutual information.");
     GetMutualInformation(vocab, candidates, &mutual_information, &status);
  }

  if (status.ok()) {
    snprintf(logmsg,
             sizeof(logmsg),
             "Write mutual information to %s",
             MUTUALINFO_DEBUG_FILE);
    log_func(logmsg);
    WritableFile *wf = WritableFile::New(MUTUALINFO_DEBUG_FILE, &status);
    for (utils::unordered_map<std::string, double>::iterator
         it = mutual_information.begin(); it != mutual_information.end();
         ++it) {
      if (!status.ok()) break;

      snprintf(line, sizeof(line), "%s %.3f", it->first.c_str(), it->second);
      wf->WriteLine(line, &status);
    }
    delete wf;
  }

  if (status.ok()) {
    log_func("Calculate final rank.");
     FinalRank(adjacent_entropy, mutual_information, result);
  }

  if (!status.ok()) {
    log_func(status.what());
  } else {
    log_func("Success!");
  }

  return status.ok();
}

}  // namespace milkcat

struct nekoneko_result_t {
  std::vector<std::pair<std::string, double> > internal_result;
};

nekoneko_result_t *nekoneko_extract(
    const char *corpus_path,
    const char *vocabulary_word_file,
    void (* log_func)(const char *msg),
    void (* progress_func)(int64_t bytes_processed,
                           int64_t file_size,
                           int64_t bytes_per_second)) {
  nekoneko_result_t *result = new nekoneko_result_t;

  bool success = milkcat::ExtractNewWord(
      corpus_path,
      vocabulary_word_file,
      &result->internal_result,
      log_func,
      progress_func);

  if (success) {
    return result;
  } else {
    delete result;
    return NULL;
  }
}

int nekoneko_result_size(nekoneko_result_t *result) {
  return result->internal_result.size();
}

const char *nekoneko_result_get_word_at(nekoneko_result_t *result, int pos) {
  return result->internal_result[pos].first.c_str();
}

double nekoneko_result_get_weight_at(nekoneko_result_t *result, int pos) {
  return result->internal_result[pos].second;
}

void nekoneko_result_destroy(nekoneko_result_t *result) {
  delete result;
}