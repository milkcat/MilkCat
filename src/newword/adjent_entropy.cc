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
// bigram_anal.cc --- Created at 2014-01-30
// extract_adjent.cc --- Created at 2014-03-18
//

#include <math.h>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include "common/model_factory.h"
#include "include/milkcat.h"
#include "parser/libmilkcat.h"
#include "utils/readable_file.h"
#include "utils/status.h"
#include "utils/thread.h"
#include "utils/utils.h"

namespace milkcat {

namespace {

struct Adjacent {
  std::map<int, int> left;
  std::map<int, int> right;
};

const int kSystemWordIdStart = 0x20000000;
const int kNotCjk = 0x30000000;

// Get a word's id from dict or system unigram index. If the word in system
// unigram index, return word-id in index + kSystemWordIdStart. If the word not
// exist, return kNotCjk
int GetWordId(const utils::unordered_map<std::string, int> &candidate_id,
              const TrieTree *index,
              const std::string &word) {
  utils::unordered_map<std::string, int>::const_iterator
  it = candidate_id.find(word);
  if (it == candidate_id.end()) {
    int system_word_id = index->Search(word.c_str());
    if (system_word_id >= 0) {
      return kSystemWordIdStart + system_word_id;
    } else {
      return kNotCjk;
    }
  } else {
    return it->second;
  }
}

// Segment a string specified by text and store it into segment_result.
void SegmentText(
      const char *text,
      Parser *parser,
      std::vector<std::string> *segment_result) {
  segment_result->clear();
  Parser::Iterator *it = parser->Parse(text);

  while (!it->End()) {
    if (it->type() == Parser::kChineseWord) {
      segment_result->push_back(it->word());
    } else {
      segment_result->push_back("-NOT-CJK-");
    }
    it->Next();
  }

  parser->Release(it);
}

// Update the word_adjacent data from words specified by line_words
void UpdateAdjacent(const std::vector<int> &line_words,
                    int candidate_size,
                    std::vector<Adjacent> *word_adjacent) {
  for (int i = 0; i < line_words.size(); ++i) {
    int word_id = line_words[i];
    if (word_id < candidate_size) {
      if (i != 0)
        word_adjacent->at(word_id).left[line_words[i - 1]] += 1;

      if (i != line_words.size() - 1)
        word_adjacent->at(word_id).right[line_words[i + 1]] += 1;
    }
  }
}

// Calculates the adjacent entropy from word's adjacent word data
// specified by adjacent
double CalculateAdjacentEntropy(const std::map<int, int> &adjacent) {
  double entropy = 0, probability;
  int total_count = 0;

  for (std::map<int, int>::const_iterator it = adjacent.begin();
       it != adjacent.end(); ++it) {
    total_count += it->second;
  }

  for (std::map<int, int>::const_iterator it = adjacent.begin();
      it != adjacent.end(); ++it) {
    probability = static_cast<double>(it->second) / total_count;
    entropy += -probability * log(probability);
  }

  return entropy;
}

// Its a thread that using bigram to segment the text from fd, and update the
// adjacent_entropy and the vocab data
class BigramAnalyzeThread: public utils::Thread {
 public:
  BigramAnalyzeThread(
      Model *model,
      const utils::unordered_map<std::string, int> &candidate_id,
      const TrieTree *index,
      ReadableFile *fd,
      int candidate_size,
      utils::Mutex *fd_mutex,
      std::vector<Adjacent> *word_adjacent,
      utils::unordered_map<std::string, int> *vocab,
      utils::Mutex *vocab_mutex,
      utils::Mutex *adjent_mutex,
      Status *status):
          model_(model),
          candidate_id_(candidate_id),
          index_(index),
          fd_(fd),
          candidate_size_(candidate_size),
          fd_mutex_(fd_mutex),
          word_adjacent_(word_adjacent),
          vocab_(vocab),
          vocab_mutex_(vocab_mutex),
          adjent_mutex_(adjent_mutex),
          status_(status) {
  }

  void Run() {
    Parser *parser = Parser::New(
        model_,
        Parser::kBigramSegmenter | Parser::kNoTagger);
    if (parser == NULL) {
      *status_ = Status::Corruption(LastError());
    }

    int buf_size = 1024 * 1024;
    char *buf = new char[buf_size];

    bool eof = false;
    std::vector<std::string> words;
    std::vector<int> word_ids;
    while (status_->ok() && !eof) {
      fd_mutex_->Lock();
      eof = fd_->Eof();
      if (!eof) fd_->ReadLine(buf, buf_size, status_);
      fd_mutex_->Unlock();

      if (status_->ok() && !eof) {
        // Using bigram model to segment the corpus
        SegmentText(buf, parser, &words);
        vocab_mutex_->Lock();
        for (int i = 0; i < words.size(); ++i) {
          utils::unordered_map<std::string, int>::iterator
          it_voc = vocab_->find(words[i]);
          if (it_voc == vocab_->end()) {
            vocab_->insert(std::pair<std::string, int>(words[i], 1));
          } else {
            it_voc->second += 1;
          }
        }
        vocab_mutex_->Unlock();

        // Now start to update the data
        word_ids.clear();
        for (int i = 0; i < words.size(); ++i) {
          word_ids.push_back(GetWordId(candidate_id_, index_, words[i]));
        }
        adjent_mutex_->Lock();
        UpdateAdjacent(word_ids, candidate_size_, word_adjacent_);
        adjent_mutex_->Unlock();
      }
    }

    delete parser;
    delete[] buf;
  }

 private:
  Model *model_;
  const utils::unordered_map<std::string, int> &candidate_id_;
  const TrieTree *index_;
  ReadableFile *fd_;
  int candidate_size_;
  utils::Mutex *fd_mutex_;
  std::vector<Adjacent> *word_adjacent_;
  utils::unordered_map<std::string, int> *vocab_;
  utils::Mutex *vocab_mutex_;
  utils::Mutex *adjent_mutex_;
  Status *status_;
};

// Thread to update progress information via calling callback function progress
class ProgressUpdateThread: public utils::Thread {
 public:
  ProgressUpdateThread(
      ReadableFile *fd,
      utils::Mutex *fd_mutex,
      bool *task_finished,
      void (* progress)(int64_t bytes_processed,
                        int64_t file_size,
                        int64_t bytes_per_second)):
          fd_(fd),
          fd_mutex_(fd_mutex),
          task_finished_(task_finished),
          progress_(progress) {
  }

  void Run() {
    int64_t file_size = fd_->Size();
    int64_t last_bytes_processed = 0,
            bytes_processed = 0;

    while (*task_finished_ == false) {
      utils::Sleep(1.0);

      last_bytes_processed = bytes_processed;
      fd_mutex_->Lock();
      bytes_processed = fd_->Tell();
      fd_mutex_->Unlock();
      progress_(bytes_processed,
                file_size,
                bytes_processed - last_bytes_processed);
    }    
  }


 private:
  ReadableFile *fd_;
  utils::Mutex *fd_mutex_;
  bool *task_finished_;
  void (* progress_)(int64_t bytes_processed,
                     int64_t file_size,
                     int64_t bytes_per_second);  
};

}  // end namespace

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
    Status *status) {
  utils::unordered_map<std::string, int> candidate_id;

  int candidate_size = candidate.size();
  std::vector<Adjacent> word_adjacent(candidate_size);

  // Put each candidate word into dictionary and assign its word-id
  for (utils::unordered_map<std::string, float>::const_iterator
       it = candidate.begin(); it != candidate.end(); ++it) {
    candidate_id.insert(std::pair<std::string, int>(
        it->first,
        candidate_id.size()));
  }

  // The model file
  // TODO: add model path for this model
  Model *model = Model::New();
  model->SetUserDictionary("candidate_cost.txt");

  const TrieTree *index = model->impl()->Index(status);

  ReadableFile *fd = NULL;
  if (status->ok()) {
    fd = ReadableFile::New(corpus_path, status);
  }

  if (status->ok()) {
    int thread_num = utils::HardwareConcurrency();
    std::vector<Status> status_vec(thread_num);
    std::vector<utils::Thread *> thread_pool;
    utils::Mutex vocab_mutex, adjent_mutex, fd_mutex;
    utils::Thread *progress_thread = NULL;
    bool task_finished = false;

    for (int i = 0; i < thread_num; ++i) {
      utils::Thread *worker_thread = new BigramAnalyzeThread(
          model,
          candidate_id,
          index,
          fd,
          candidate_size,
          &fd_mutex,
          &word_adjacent,
          vocab,
          &vocab_mutex,
          &adjent_mutex,
          &status_vec[i]);
      worker_thread->Start();
      thread_pool.push_back(worker_thread);
    }

    if (progress) {
      progress_thread = new ProgressUpdateThread(
          fd,
          &fd_mutex,
          &task_finished,
          progress);
      progress_thread->Start();           
    }

    // Synchronizing all threads
    while (thread_pool.size() > 0) {
      thread_pool.back()->Join();
      delete thread_pool.back();
      thread_pool.pop_back();
    }
    task_finished = true;
    if (progress) {
      progress_thread->Join();
      delete progress_thread;
      progress_thread = NULL;
    }

    // Set the status
    for (int i = 0; i < status_vec.size(); ++i) {
      if (!status_vec[i].ok()) *status = status_vec[i];
    }
  }

  if (status->ok()) {
    std::vector<std::string> id_to_str(candidate_id.size());
    for (utils::unordered_map<std::string, int>::iterator
         it = candidate_id.begin(); it != candidate_id.end(); ++it) {
      id_to_str[it->second] = it->first;
    }

    // Calculate the candidates' adjacent entropy and store in adjacent_entropy
    std::string word;
    for (int i = 0; i < word_adjacent.size(); ++i) {
      double left_entropy = CalculateAdjacentEntropy(word_adjacent[i].left);
      double right_entropy = CalculateAdjacentEntropy(word_adjacent[i].right);
      word = id_to_str[i];
      adjacent_entropy->insert(std::pair<std::string, double>(
          word,
          std::min(left_entropy, right_entropy)));
    }
  }

  delete fd;
  delete model;
}

}  // namespace milkcat