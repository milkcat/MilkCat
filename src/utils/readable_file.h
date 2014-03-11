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
// random_access_file.h --- Created at 2014-01-28
// readable_file.h --- Created at 2014-02-03
//

#ifndef SRC_UTILS_READABLE_FILE_H_
#define SRC_UTILS_READABLE_FILE_H_

#include <stdio.h>
#include <stdint.h>
#include <string>
#include "utils/status.h"

namespace milkcat {

// open a random access file for read
class ReadableFile {
 public:
  static ReadableFile *New(const char *file_path, Status *status);
  ~ReadableFile();

  // Read n bytes (size) from file and put to *ptr
  bool Read(void *ptr, int size, Status *status);

  // Read an type T from file
  template<typename T>
  bool ReadValue(T *data, Status *status) {
    return Read(data, sizeof(T), status);
  }

  // Read a line from file, if failed return false. NOTE: The line string 
  // contains the CR or CR-LF characters 
  bool ReadLine(char *buf, int size, Status *status);

  bool Eof() { return Tell() >= size_; }

  // Get current position in file
  int64_t Tell();

  // Get the file size
  int64_t Size() { return size_; }

 private:
  FILE *fd_;
  int64_t size_;
  std::string file_path_;

  ReadableFile();
};

}  // namespace milkcat

#endif  // SRC_UTILS_READABLE_FILE_H_
