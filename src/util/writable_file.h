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
// writable_file.h --- Created at 2014-02-03
//

#ifndef SRC_UTIL_WRITABLE_FILE_H_
#define SRC_UTIL_WRITABLE_FILE_H_

#include <stdio.h>
#include <string>
#include "util/status.h"

namespace milkcat {

class WritableFile {
 public:
  // Open a file for write. On success, return an instance of WritableFile.
  // On failed, set status != Status::OK()
  static WritableFile *New(const char *path, Status *status);
  ~WritableFile();

  // Writes a line to file
  void WriteLine(const char *line, Status *status);

  // Writes data to file
  void Write(const void *data, int size, Status *status);

  // Write an type T to file
  template<typename T>
  void WriteValue(const T &data, Status *status) {
    Write(&data, sizeof(data), status);
  }

 private:
  FILE *fd_;
  std::string file_path_;

  WritableFile();
};

}  // namespace milkcat

#endif  // SRC_UTIL_WRITABLE_FILE_H_
