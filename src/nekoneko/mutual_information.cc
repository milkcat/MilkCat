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
// mutual_information.cc --- Created at 2014-02-06
//

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <map>
#include <string>
#include "utils/utils.h"
#include "utils/writable_file.h"
#include "milkcat/libmilkcat.h"
#include "milkcat/bigram_segmenter.h"

namespace milkcat {

void GetMutualInformation(
    const utils::unordered_map<std::string, int> &bigram_vocab,
    const utils::unordered_map<std::string, float> &candidate,
    utils::unordered_map<std::string, double> *mutual_information,
    Status *status) {

  char line[1024];

  // Calculate total_frequency and frequency of candidate words
  utils::unordered_map<std::string, int> candidate_frequencies;
  typedef utils::unordered_map<std::string, int>::const_iterator msi_citer;
  typedef utils::unordered_map<std::string, float>::const_iterator msf_citer;
  int total_frequency = 0;
  for (msi_citer it = bigram_vocab.begin(); it != bigram_vocab.end(); ++it) {
    total_frequency += it->second;
    msf_citer it_candi = candidate.find(it->first);
    if (it_candi != candidate.end()) {
      candidate_frequencies[it->first] = it->second;
    }
  }

  // Prepare the user dictionary
  WritableFile *fd = WritableFile::New("bigram_vocab.txt", status);
  for (msi_citer it = bigram_vocab.begin(); it != bigram_vocab.end(); ++it) {
    if (!status->ok()) break;
    snprintf(line,
             sizeof(line),
             "%s %.5lf",
             it->first.c_str(),
             -log(static_cast<double>(it->second) / total_frequency));
    fd->WriteLine(line, status);
  }
  delete fd;

  // Start to calculate the mutual information for candidates
  BigramSegmenter *segmenter;
  milkcat_model_t *model;
  milkcat_t *analyzer;
  milkcat_cursor_t *cursor;
  
  if (status->ok()) {
    model = milkcat_model_new(NULL);
    milkcat_model_set_userdict(model, "bigram_vocab.txt");
    analyzer = milkcat_new(model, BIGRAM_SEGMENTER);
    if (analyzer == NULL)
      *status = Status::RuntimeError(milkcat_last_error());
  }

  if (status->ok()) {
    segmenter = static_cast<BigramSegmenter *>(analyzer->segmenter);
    typedef utils::unordered_map<std::string, int>::iterator msi_iter;
    for (msi_iter it = candidate_frequencies.begin(); 
         it != candidate_frequencies.end();
         ++it) {
      const char *word = it->first.c_str();
      int term_id = segmenter->GetTermId(word);
      assert(term_id > 0);
      segmenter->ClearAllDisabledTermIds();
      segmenter->AddDisabledTermId(term_id);

      milkcat_item_t *item = milkcat_analyze(analyzer, word);
      while (item) {
        item = milkcat_analyze(analyzer, NULL);
      }

      double word_cost = -log(
          static_cast<double>(it->second) / total_frequency);
      double bigram_cost = segmenter->RecentSegCost();

      mutual_information->insert(std::pair<std::string, double>(
          it->first,
          bigram_cost - word_cost));
    }
  }

  milkcat_destroy(analyzer);
  milkcat_model_destroy(model);
}

}  // namespace milkcat