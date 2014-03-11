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
// candidate.h --- Created at 2014-02-03
//

#ifndef SRC_NEKO_CANDIDATE_H_
#define SRC_NEKO_CANDIDATE_H_

#include <vector>
#include <unordered_map>
#include <string>
#include "utils/status.h"

namespace milkcat {

constexpr int kDefaultThresFreq = -1;

// Get the candidate from crf segmentation vocabulary specified by crf_vocab.
// Returns a map the key is the word, and the value is its cost in unigram
std::unordered_map<std::string, float> GetCandidate(
    const char *model_path,
    const char *vocabulary_path,
    const std::unordered_map<std::string, int> &crf_vocab,
    int total_count,
    int (* log_func)(const char *message),
    Status *status,
    int thres_freq = kDefaultThresFreq);

}  // namespace milkcat

#endif  // SRC_NEKO_CANDIDATE_H_
