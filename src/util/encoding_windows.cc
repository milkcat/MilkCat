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
// encoding.cc --- Created at 2015-02-20
//

#include "util/encoding.h"

#include <windows.h>

namespace milkcat {

class Encoding::Impl {
 public:
  Impl() {
    buffer_size_ = 1024;
    wchar_buffer_ = new WCHAR[buffer_size_];
  }

  ~Impl() {
    delete[] wchar_buffer_;
    wchar_buffer_ = NULL;
  }

  bool GBKToUTF8(const char *input, char *output, int output_size) {
    return Convert(936, CP_UTF8, input, output, output_size);
  }

  bool UTF8ToGBK(const char *input, char *output, int output_size) {
    return Convert(CP_UTF8, 936, input, output, output_size);
  }

 private:
  WCHAR *wchar_buffer_;
  int buffer_size_;

  // Converts string between different code pages 
  bool Convert(int from_codepage,
               int to_codepage,
               const char *input,
               char *output,
               int output_size) {
    int required = MultiByteToWideChar(from_codepage, 0, input, -1, NULL, 0);
    if (buffer_size_ < required) {
      delete[] wchar_buffer_;
      wchar_buffer_ = new WCHAR[required];
      buffer_size_ = required;
    }
    MultiByteToWideChar(from_codepage, 0, input, -1, wchar_buffer_, buffer_size_);
    int bytes_written = WideCharToMultiByte(
      to_codepage, 0, wchar_buffer_, -1, output, output_size, NULL, NULL);
    if (bytes_written == 0) {
      return false;
    } else {
      return true;
    }
  }
};

Encoding::Encoding(): impl_(new Impl()) {
}

Encoding::~Encoding() {
  delete impl_;
  impl_ = NULL;
}

bool Encoding::GBKToUTF8(const char *input, char *output, int output_size) {
  return impl_->GBKToUTF8(input, output, output_size);
}

bool Encoding::UTF8ToGBK(const char *input, char *output, int output_size) {
  return impl_->UTF8ToGBK(input, output, output_size);
}

}  // namespace milkcat


