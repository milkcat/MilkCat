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
// newword.cc --- Created at 2014-06-24
//

#include "include/milkcat.h"
#include "newword/newword.h"

#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>
#include "parser/libmilkcat.h"
#include "utils/log.h"
#include "utils/status.h"
#include "utils/utils.h"
#include "utils/writable_file.h"

namespace milkcat {

const char *MUTUALINFO_DEBUG_FILE = "mi.txt";
const char *ADJENT_DEBUG_FILE = "adjent.txt";

class Newword::Impl {
 public:
  Impl(const char *model_dir);
  ~Impl();

  Iterator *Extract(const char *filename);
  void Release(Iterator *it);

  void set_vocabulary_file(const char *vocabulary_filename) {
    vocabulary_filename_ = vocabulary_filename;
  }
  void set_log_function(LogFunction log_func) {
    log_function_ = log_func;
  }
  void set_progress_function(ProgressFunction progress_func) {
    progress_function_ = progress_func;
  }

 private:
  std::string vocabulary_filename_;
  std::string model_dir_;
  std::vector<Iterator *> iterator_pool_;
  int iterator_alloced_;

  LogFunction log_function_;
  ProgressFunction progress_function_;

  // Using log_function_ to display an message
  void Log(const LogUtil &message);
};

class Newword::Iterator::Impl {
 public:
  Impl();

  bool End() { return result_pos_ >= internal_result_.size(); }
  void Next() { if (!End()) ++result_pos_; }

  const char *word() {
    return internal_result_[result_pos_].first.c_str();
  }

  double weight() {
    return internal_result_[result_pos_].second;
  }

  std::vector<std::pair<std::string, double> > *
  result() { return &internal_result_; }

  // Reset the iterator
  void Reset() {
    internal_result_.clear();
    result_pos_ = 0;
  }

 private:
  std::vector<std::pair<std::string, double> > internal_result_;
  int result_pos_;
};

Newword::Impl::Impl(const char *model_dir):
    model_dir_(model_dir? model_dir: MODEL_PATH),
    vocabulary_filename_(""),
    log_function_(NULL),
    progress_function_(NULL),
    iterator_alloced_(0) {
}

Newword::Impl::~Impl() {
  for (std::vector<Iterator *>::iterator
       it = iterator_pool_.begin(); it != iterator_pool_.end(); ++it) {
    delete *it;
  }
}

inline void Newword::Impl::Log(const LogUtil &message) {
  if (log_function_) log_function_(message.GetString().c_str());
}

Newword::Iterator *Newword::Impl::Extract(const char *filename) {
  Status status;
  int total_count = 0;

  Log(LogUtil() << "Segment corpus " << filename << " with CRF model.");

  utils::unordered_map<std::string, int> vocab;
  if (status.ok()) 
    CrfVocabulary(filename,
                  &total_count,
                  &vocab,
                  progress_function_,
                  &status);    


  if (status.ok()) {
    Log(LogUtil() << "OK, " << total_count
                  << " words in corpus, vocabulary size is " << vocab.size());
    Log(LogUtil() << "Get candidates from vocabulary.");
  }

  utils::unordered_map<std::string, float> candidates;
  if (status.ok()) {
    std::string model_path = MODEL_PATH;
    model_path += "person_name.maxent";

    // TODO: Add support for vocabulary_filename_ here
    GetCandidate(
        model_path.c_str(),
        NULL,
        vocab,
        total_count,
        &candidates,
        log_function_,
        &status,
        kUseDefaultThresFreq);
  }

  if (status.ok()) {
    Log(LogUtil() << "Get " << candidates.size() << "candidates. "
                  << " Write to candidate_cost.txt");
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

  if (status.ok()) {
    Log(LogUtil() << "Parse " << filename << " with bigram segmenter.");
  }

  if (status.ok()) {
    ExtractAdjacent(candidates,
                    filename,
                    &adjacent_entropy,
                    &vocab,
                    progress_function_,
                    &status);
  }

  if (status.ok()) {
    Log(LogUtil() << "Write adjacent entropy to " << ADJENT_DEBUG_FILE);

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
    Log(LogUtil() << "Calculate mutual information.");
    GetMutualInformation(vocab, candidates, &mutual_information, &status);
  }

  if (status.ok()) {
    Log(LogUtil() << "Write mutual information to " << MUTUALINFO_DEBUG_FILE);

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

  Newword::Iterator *it;
  if (iterator_pool_.size() == 0) {
    ASSERT(iterator_alloced_ < 1024,
           "Too many Newword::Iterator allocated without Newword::Release.");
    it = new Newword::Iterator();
    iterator_alloced_++;
  } else {
    it = iterator_pool_.back();
    iterator_pool_.pop_back();
  }

  it->impl()->Reset();

  if (status.ok()) {
    Log(LogUtil() << "Rank.");
    NewwordRank(adjacent_entropy, mutual_information, it->impl()->result());
  }

  if (!status.ok()) {
    Log(LogUtil() << status.what());
    global_status = status;
    Release(it);
    return NULL;
  } else {
    Log(LogUtil() << "Success!");
    return it;
  }
}

void Newword::Impl::Release(Newword::Iterator *it) {
  iterator_pool_.push_back(it);
}

Newword::Iterator::Impl::Impl(): result_pos_(0) {
}


Newword::Newword(): impl_(NULL) {
  
}
Newword::~Newword() {
  delete impl_;
  impl_ = NULL;
}
Newword *Newword::New(const char *model_dir) {
  Newword *self = new Newword();
  self->impl_ = new Newword::Impl(model_dir);

  if (!self->impl_) {
    delete self;
    return NULL;
  } else {
    return self;
  }
}
Newword::Iterator *Newword::Extract(const char *filename) {
  return impl_->Extract(filename);
}
void Newword::Release(Iterator *it) { return impl_->Release(it); }
void Newword::set_vocabulary_file(const char *vocabulary_filename) {
  impl_->set_vocabulary_file(vocabulary_filename);
}
void Newword::set_log_function(LogFunction log_func) {
  impl_->set_log_function(log_func);
}
void Newword::set_progress_function(ProgressFunction progress_func) {
  impl_->set_progress_function(progress_func);
}

Newword::Iterator::Iterator(): impl_(new Impl()) {
}
Newword::Iterator::~Iterator() {
  delete impl_;
  impl_ = NULL;
}
bool Newword::Iterator::End() { return impl_->End(); }
void Newword::Iterator::Next() { impl_->Next(); }
const char *Newword::Iterator::word() { return impl_->word(); }
double Newword::Iterator::weight() { return impl_->weight(); }

}  // namespace milkcat
