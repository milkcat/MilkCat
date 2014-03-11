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
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <string>
#include "neko/candidate.h"
#include "neko/crf_vocab.h"
#include "neko/bigram_anal.h"
#include "neko/mutual_information.h"
#include "neko/final_rank.h"
#include "utils/utils.h"
#include "utils/writable_file.h"

namespace milkcat {

constexpr const char *MUTUALINFO_DEBUG_FILE = "mi.txt";
constexpr const char *ADJENT_DEBUG_FILE = "adjent.txt";

void DisplayProgress(int64_t bytes_processed,
                     int64_t file_size,
                     int64_t bytes_per_second) {
  fprintf(stderr,
          "\rprogress %dMB/%dMB -- %2.1f%% %.3fMB/s",
          static_cast<int>(bytes_processed / (1024 * 1024)),
          static_cast<int>(file_size / (1024 * 1024)),
          100.0 * bytes_processed / file_size,
          bytes_per_second / static_cast<double>(1024 * 1024));
}

int main(int argc, char **argv) {
  Status status;
  int total_count = 0;
  char errmsg[1024],
       vocabulary_file[1024] = "",
       output_file[1024] = "";
  int c;

  while ((c = getopt(argc, argv, "u:o:")) != -1) {
    switch (c) {
      case 'u':
        strlcpy(vocabulary_file, optarg, sizeof(errmsg));
        break;

      case 'o':
        strlcpy(output_file, optarg, sizeof(errmsg));
        break;

      case ':':
        sprintf(errmsg, "Option -%c requires an operand", optopt);
        status = Status::Info(errmsg);
        break;

      case '?':
        sprintf(errmsg, "Unrecognized option: -%c", optopt);
        status = Status::Info(errmsg);
        break;
    }
  }

  printf("Segment corpus %s with CRF model.\n", argv[optind]);
  std::unordered_map<std::string, int> vocab;
  if (status.ok()) {
    vocab = GetCrfVocabulary(
        argv[optind],
        &total_count,
        DisplayProgress,
        &status);    
  }

  if (status.ok()) {
    printf("\nOK, %d words in corpus, vocabulary size is"
           " %d\nGet candidates from vocabulary.\n",
           total_count,
           static_cast<int>(vocab.size()));
  }

  std::unordered_map<std::string, float> candidates;
  if (status.ok()) {
    std::string model_path = MODEL_PATH;
    model_path += "person_name.maxent";
    candidates = GetCandidate(
        model_path.c_str(),
        *vocabulary_file != '\0' ? vocabulary_file : nullptr,
        vocab,
        total_count,
        puts,
        &status);
  }

  if (status.ok()) {
    printf("Get %d candidates. Write to candidate_cost.txt\n",
           static_cast<int>(candidates.size()));
  }

  WritableFile *fd = nullptr;
  if (status.ok()) fd = WritableFile::New("candidate_cost.txt", &status);

  char line[1024];
  if (status.ok()) {
    for (auto &x : candidates) {
      snprintf(line, sizeof(line), "%s %.5f", x.first.c_str(), x.second);
      fd->WriteLine(line, &status);
    }
  }

  delete fd;

  // Clear the vocab
  vocab = std::unordered_map<std::string, int>();

  std::unordered_map<std::string, double> adjacent_entropy;
  std::unordered_map<std::string, double> mutual_information;

  if (status.ok()) {
    printf("Analyze %s with bigram segmentation.\n", argv[optind]);
    BigramAnalyze(candidates,
                  argv[optind],

                  &adjacent_entropy,
                  &vocab,
                  DisplayProgress,
                  &status);
  }
  puts("");

  if (status.ok()) {
    printf("Write adjacent entropy to %s\n", ADJENT_DEBUG_FILE);
    WritableFile *wf = WritableFile::New(ADJENT_DEBUG_FILE, &status);
    for (auto &x : adjacent_entropy) {
      if (!status.ok()) break;

      snprintf(line, sizeof(line), "%s %.3f", x.first.c_str(), x.second);
      wf->WriteLine(line, &status);
    }
    delete wf;
  }

  if (status.ok()) {
    printf("Calculate mutual information.\n");
    mutual_information = GetMutualInformation(vocab, candidates, &status);
  }

  if (status.ok()) {
    printf("Write mutual information to %s\n", MUTUALINFO_DEBUG_FILE);
    WritableFile *wf = WritableFile::New(MUTUALINFO_DEBUG_FILE, &status);
    for (auto &x : mutual_information) {
      if (!status.ok()) break;

      snprintf(line, sizeof(line), "%s %.3f", x.first.c_str(), x.second);
      wf->WriteLine(line, &status);
    }
    delete wf;
  }

  std::vector<std::pair<std::string, double>> final_rank;
  if (status.ok()) {
    printf("Calculate final rank.\n");
    final_rank = FinalRank(adjacent_entropy, mutual_information);
  }

  if (status.ok()) {
    printf("Write result to %s\n", output_file);
    WritableFile *wf = WritableFile::New(output_file, &status);
    for (auto &x : final_rank) {
      if (!status.ok()) break;

      snprintf(line, sizeof(line), "%s %.3f", x.first.c_str(), x.second);
      wf->WriteLine(line, &status);
    }
    delete wf;
  }

  if (!status.ok()) {
    printf("%s\n", status.what());
  } else {
    puts("Success!");
  }

  return 0;
}

}  // namespace milkcat

int main(int argc, char **argv) {
  return milkcat::main(argc, argv);
}