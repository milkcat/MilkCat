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
// random_access_file.cc --- Created at 2014-01-28
// readable_file.cc --- Created at 2014-02-03
//

#include "utils/readable_file.h"
#include <stdio.h>
#include <string>
#include "utils/status.h"

namespace milkcat {

ReadableFile *ReadableFile::New(const char *file_path, Status *status) {
  ReadableFile *self = new ReadableFile();
  self->file_path_ = file_path;
  std::string msg;

  if ((self->fd_ = fopen(file_path, "rb")) != NULL) {
    fseek(self->fd_, 0, SEEK_END);
    self->size_ = ftello(self->fd_);
    fseek(self->fd_, 0, SEEK_SET);
    return self;

  } else {
    std::string msg("failed to open ");
    msg += file_path;
    *status = Status::IOError(msg.c_str());
    return NULL;
  }
}

ReadableFile::ReadableFile(): fd_(NULL), size_(0) {}

bool ReadableFile::Read(void *ptr, int size, Status *status) {
  if (1 != fread(ptr, size, 1, fd_)) {
    std::string msg("failed to read from ");
    msg += file_path_;
    *status = Status::IOError(msg.c_str());
    return false;
  } else {
    return true;
  }
}

bool ReadableFile::ReadLine(char *ptr, int size, Status *status) {
  if (NULL == fgets(ptr, size, fd_)) {
    std::string msg("failed to read from ");
    msg += file_path_;
    *status = Status::IOError(msg.c_str());
    return false;
  } else {
    return true;
  }
}

ReadableFile::~ReadableFile() {
  if (fd_ != NULL) fclose(fd_);
}

int64_t ReadableFile::Tell() {
  return ftello(fd_);
}

}  // namespace milkcat
