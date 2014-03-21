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
// thread_posix.cc --- Created at 2014-03-17
//

#include <assert.h>
#include <pthread.h>
#include "utils/thread.h"
#include "utils/utils.h"

namespace milkcat {
namespace utils {

void *ThreadFunc(void *args) {
  Thread *thread = static_cast<Thread *>(args);
  thread->Run();

  return NULL;
}

class Thread::ThreadImpl {
 public:
  ThreadImpl(Thread *thread): has_joined_(false),
                              has_started_(false),
                              thread_(thread) {
  }

  ~ThreadImpl() {
    assert(has_joined_);
  }

  void Start() {
    assert(!has_started_);
    int rc = pthread_create(&thread_handle_, NULL, ThreadFunc, thread_);
    assert(rc == 0);
    has_started_ = true;
  }

  void Join() {
    void *status = NULL;
    assert(!has_joined_);
    int rc = pthread_join(thread_handle_, &status);
    assert(rc == 0);
    has_joined_ = true;
  }

 private:
  pthread_t thread_handle_;
  Thread *thread_;
  bool has_joined_;
  bool has_started_;

  DISALLOW_COPY_AND_ASSIGN(ThreadImpl);
};

Thread::Thread(): impl_(new Thread::ThreadImpl(this)) {}
Thread::~Thread() {
  delete impl_;
  impl_ = NULL;
}

void Thread::Start() { impl_->Start(); }
void Thread::Join() { impl_->Join(); }

}  // namespace utils
}  // namespace milkcat

