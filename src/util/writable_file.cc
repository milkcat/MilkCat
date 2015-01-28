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
// writable_file.cc --- Created at 2014-02-04
//

#include "util/writable_file.h"

#include <string>

namespace milkcat {

WritableFile::WritableFile(): fd_(NULL) {
}

WritableFile::~WritableFile() {
  if (fd_ != NULL) fclose(fd_);
}

WritableFile *WritableFile::New(const char *path, Status *status) {
  std::string error_message;
  WritableFile *self = new WritableFile();
  self->file_path_ = path;

  self->fd_ = fopen(path, "wb");
  if (self->fd_ == NULL) {
    error_message = std::string("Unable to open ") + path + " for write.";
    *status = Status::IOError(error_message.c_str());
  }

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

void WritableFile::WriteLine(const char *line, Status *status) {
  std::string error_message;
  int r = fputs(line, fd_);
  int r2 = fputc('\n', fd_);

  if (r < 0 || r2 == EOF) {
    error_message = std::string("Failed to write to ") + file_path_;
    *status = Status::IOError(error_message.c_str());
  }
}

void WritableFile::Write(const void *data, int size, Status *status) {
  std::string error_message;

  if (fwrite(data, size, 1, fd_) < 1) {
    error_message = std::string("Failed to write to ") + file_path_;
    *status = Status::IOError(error_message.c_str());
  }
}

}  // namespace milkcat
