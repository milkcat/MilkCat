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
// get_vocabulary.cc --- Created at 2014-02-21
// word_frequency_count.cc -- Created at 2014-09-02
//

#include "common/word_frequency_count.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "include/milkcat.h"
#include "utils/mutex.h"
#include "utils/readable_file.h"
#include "utils/status.h"
#include "utils/utils.h"
#include "utils/thread.h"

namespace milkcat {

class SegmentThread: public utils::Thread {
 public:
  SegmentThread(
    const Parser::Options &options,
    ReadableFile *fd,
    utils::Mutex *fd_mutex,
    utils::unordered_map<std::string, int> *vocab,
    int *total_count,
    utils::Mutex *vocab_mutex,
    Status *status): 
      options_(options),
      fd_(fd),
      fd_mutex_(fd_mutex),
      vocab_(vocab),
      total_count_(total_count),
      vocab_mutex_(vocab_mutex),
      status_(status) {
  }

  void Run() {
    int buf_size = 1024 * 1024;
    char *buf = new char[buf_size];

    // Creates an parser from model
    Parser *parser = Parser::New(options_);
    Parser::Iterator *it = new Parser::Iterator();
    if (parser == NULL) *status_ = Status::Corruption(LastError());

    bool eof = false;
    std::vector<std::string> words;
    while (status_->ok() && !eof) {
      // Read a line from corpus
      fd_mutex_->Lock();
      eof = fd_->Eof();
      if (!eof) fd_->ReadLine(buf, buf_size, status_);
      fd_mutex_->Unlock();

      // Segment the line and store the results into words
      if (status_->ok() && !eof) {
        words.clear();
        parser->Parse(buf, it);
        while (!it->End()) {
          if (it->type() == Parser::kChineseWord)
            words.push_back(it->word());
          else
            words.push_back("-NOT-CJK-");
          it->Next();
        }
      }

      // Update vocab and total_count with words
      typedef utils::unordered_map<std::string, int>::iterator voc_itertype;
      typedef std::vector<std::string>::iterator word_itertype;
      if (status_->ok() && !eof) {
        vocab_mutex_->Lock();
        for (word_itertype it = words.begin(); it != words.end(); ++it) {
          voc_itertype it_voc = vocab_->find(*it);
          if (it_voc != vocab_->end()) {
            it_voc->second += 1;
          } else {
            vocab_->insert(std::pair<std::string, int>(*it, 1));
          }
        }
        *total_count_ += words.size();
        vocab_mutex_->Unlock();
      }
    }
    
    delete parser;
    delete it;
    delete[] buf;
  }

 private:
  Parser::Options options_;
  ReadableFile *fd_;
  utils::Mutex *fd_mutex_;
  utils::unordered_map<std::string, int> *vocab_;
  int *total_count_;
  utils::Mutex *vocab_mutex_;
  Status *status_;  
};

struct ProgressThreadArgs {
  ReadableFile *fd;
  utils::Mutex *fd_mutex;
  bool *task_finished;
  void (* progress)(int64_t bytes_processed,
                    int64_t file_size,
                    int64_t bytes_per_second);
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
      // Sleep 1 second
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

// Segment the corpus from path and count the occurrence frequencies of each
// word. Returns the number of used words. 
// If any errors occured, status is set to a value != Status::OK()
int CountWordFrequencyFromFile(
    const char *path,
    const Parser::Options &options,
    int n_threads,
    utils::unordered_map<std::string, int> *vocab,
    void (* progress)(int64_t bytes_processed,
                      int64_t file_size,
                      int64_t bytes_per_second),
    Status *status) {
  
  int total_count = 0;

  ReadableFile *fd = NULL;
  if (status->ok()) fd = ReadableFile::New(path, status);

  if (status->ok()) {
    // Thread number = CPU core number
    std::vector<Status> status_vec(n_threads);
    std::vector<utils::Thread *> thread_pool;
    utils::Thread *progress_thread = NULL;
    utils::Mutex fd_mutex, vocab_mutex;
    bool task_finished = false;

    for (int i = 0; i < n_threads; ++i) {
      utils::Thread *worker_thread = new SegmentThread(
          options,
          fd,
          &fd_mutex,
          vocab,
          &total_count,
          &vocab_mutex,
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

    // Synchronizing all threads, call progress function per second
    while (thread_pool.size() != 0) {
      thread_pool.back()->Join();
      delete thread_pool.back();
      thread_pool.pop_back();
    }

    task_finished = true;
    if (progress) {
      progress_thread->Join();
      delete progress_thread;
    } 

    // Set the status
    std::vector<Status>::iterator it;
    for (it = status_vec.begin(); it != status_vec.end(); ++it) {
      if (!it->ok()) *status = *it;
    }
  }

  delete fd;
  return total_count;
}

}  // namespace milkcat