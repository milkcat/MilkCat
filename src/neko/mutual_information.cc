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

#include "neko/mutual_information.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <map>
#include <string>
#include <unordered_map>
#include "utils/writable_file.h"
#include "milkcat/libmilkcat.h"
#include "milkcat/bigram_segmenter.h"

namespace milkcat {

std::unordered_map<std::string, double> GetMutualInformation(
    const std::unordered_map<std::string, int> &bigram_vocab,
    const std::unordered_map<std::string, float> &candidate,
    Status *status) {

  char line[1024];
  std::unordered_map<std::string, double> mutual_information;

  // Calculate total_frequency and frequency of candidate words
  std::unordered_map<std::string, int> candidate_frequencies;
  int total_frequency = 0;
  for (auto &x : bigram_vocab) {
    total_frequency += x.second;
    auto it = candidate.find(x.first);
    if (it != candidate.end()) {
      candidate_frequencies[x.first] = x.second;
    }
  }

  // Prepare the user dictionary
  WritableFile *fd = WritableFile::New("bigram_vocab.txt", status);
  for (auto &x : bigram_vocab) {
    if (!status->ok()) break;
    snprintf(line,
             sizeof(line),
             "%s %.5lf",
             x.first.c_str(),
             -log(static_cast<double>(x.second) / total_frequency));
    fd->WriteLine(line, status);
  }
  delete fd;

  // Start to calculate the mutual information for candidates
  BigramSegmenter *segmenter;
  milkcat_model_t *model;
  milkcat_t *analyzer;
  milkcat_cursor_t *cursor;
  milkcat_item_t item;
  if (status->ok()) {
    model = milkcat_model_new(nullptr);
    milkcat_model_set_userdict(model, "bigram_vocab.txt");
    analyzer = milkcat_new(model, BIGRAM_SEGMENTER);
    cursor = milkcat_cursor_new();
    if (analyzer == nullptr)
      *status = Status::RuntimeError(milkcat_last_error());
  }

  if (status->ok()) {
    segmenter = static_cast<BigramSegmenter *>(analyzer->segmenter);
    for (auto &x : candidate_frequencies) {
      const char *word = x.first.c_str();
      int term_id = segmenter->GetTermId(word);
      assert(term_id > 0);
      segmenter->ClearAllDisabledTermIds();
      segmenter->AddDisabledTermId(term_id);

      milkcat_analyze(analyzer, cursor, word);
      while (milkcat_cursor_get_next(cursor, &item)) {}

      double word_cost = -log(static_cast<double>(x.second) / total_frequency);
      double bigram_cost = segmenter->RecentSegCost();

      mutual_information.emplace(x.first, bigram_cost - word_cost);
    }
  }

  milkcat_destroy(analyzer);
  milkcat_cursor_destroy(cursor);
  milkcat_model_destroy(model);

  return mutual_information;
}

}  // namespace milkcat
