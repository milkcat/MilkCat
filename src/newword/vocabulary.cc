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
// crf_vocab.cc --- Created at 2014-01-28
// get_crf_vocabulary.cc --- Created at 2014-03-20
// vocabulary.cc --- Created at 2014-06-24
//

#include "newword/newword.h"

#include <string>
#include "include/milkcat.h"
#include "utils/readable_file.h"
#include "utils/status.h"
#include "utils/utils.h"

namespace milkcat {


// Segment the corpus from path and return the vocabulary of chinese words.
// If any errors occured, status is not Status::OK()
void CrfVocabulary(
    const char *path,
    int *total_count,
    utils::unordered_map<std::string, int> *crf_vocab,
    void (* progress)(int64_t bytes_processed,
                      int64_t file_size,
                      int64_t bytes_per_second),
    Status *status) {

  utils::unordered_map<std::string, int> vocab;

  // TODO: put model dir for this model
  Model *model = Model::New();

  *total_count = GetVocabularyFromFile(
      path,
      model,
      Parser::kCrfSegmenter | Parser::kNoTagger,
      utils::HardwareConcurrency(),
      crf_vocab,
      progress,
      status);

  delete model;
}

}  // namespace milkcat