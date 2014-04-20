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
// libnekoneko.h --- Created at 2014-03-18
//

#ifndef SRC_NEKONEKO_LIBNEKONEKO_H_
#define SRC_NEKONEKO_LIBNEKONEKO_H_

#include <stdint.h>
#include <vector>
#include "common/get_vocabulary.h"
#include "include/milkcat.h"
#include "utils/utils.h"

namespace milkcat {

const int kUseDefaultThresFreq = -1;

// Segment the corpus from path and return the vocabulary of chinese words.
// If any errors occured, status is not Status::OK()
void GetCrfVocabulary(
    const char *path,
    int *total_count,
    utils::unordered_map<std::string, int> *crf_vocab,
    void (* progress)(int64_t bytes_processed,
                      int64_t file_size,
                      int64_t bytes_per_second),
    Status *status);

// Get the candidate from crf segmentation vocabulary specified by crf_vocab.
// Set candidates a map the key is the word, and the value is its cost in 
// unigram.
void GetCandidate(
    const char *model_path,
    const char *vocabulary_path,
    const utils::unordered_map<std::string, int> &crf_vocab,
    int total_count,
    utils::unordered_map<std::string, float> *candidates,
    void (* log_func)(const char *message),
    Status *status,
    int thres_freq);

// Use bigram segmentation to analyze a corpus. Candidate to analyze is
// specified by candidate, and the corpus is specified by corpus_path. It would
// use a temporary file called 'candidate_cost.txt' as user dictionary file for
// MilkCat. On success, stores the adjecent entropy in adjacent_entropy, and
// stores the vocabulary of segmentation's result in vocab. On failed set
// status != Status::OK()
void ExtractAdjacent(
    const utils::unordered_map<std::string, float> &candidate,
    const char *corpus_path,
    utils::unordered_map<std::string, double> *adjacent_entropy,
    utils::unordered_map<std::string, int> *vocab,
    void (* progress)(int64_t bytes_processed,
                     int64_t file_size,
                     int64_t bytes_per_second),
    Status *status);

void GetMutualInformation(
    const utils::unordered_map<std::string, int> &bigram_vocab,
    const utils::unordered_map<std::string, float> &candidate,
    utils::unordered_map<std::string, double> *mutual_information,
    Status *status);

void FinalRank(
    const utils::unordered_map<std::string, double> &adjecent_entropy,
    const utils::unordered_map<std::string, double> &mutual_information,
    std::vector<std::pair<std::string, double> > *final_result,
    double remove_ratio = 0.1,
    double alpha = 0.6);

}  // namespace milkcat

#endif  // SRC_NEKONEKO_NEKONEKO_H_
