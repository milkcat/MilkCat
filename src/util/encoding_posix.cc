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
// encoding_posix.cc --- Created at 2015-02-21
//

#include "util/encoding.h"

#include <iconv.h>
#include "util/util.h"

namespace milkcat {

class Encoding::Impl {
 public:
  Impl() {
    iconv_gbk_to_utf8_ = iconv_open("UTF-8", "GBK//IGNORE");
    MC_ASSERT(iconv_gbk_to_utf8_ >= 0,
              "unable to open gbk to utf8 converter");

    iconv_utf8_to_gbk_ = iconv_open("GBK", "UTF-8//IGNORE");
    MC_ASSERT(iconv_utf8_to_gbk_ >= 0,
              "unable to open utf8 to gbk converter");
  }

  ~Impl() {
    iconv_close(iconv_gbk_to_utf8_);
    iconv_close(iconv_utf8_to_gbk_);
  }

  bool GBKToUTF8(const char *input, char *output, int output_size) {
    size_t input_size = strlen(input);
    size_t u_output_size = output_size - 1;
    size_t nconv = iconv(
        iconv_gbk_to_utf8_,
        const_interoperable<char **>(&input),
        &input_size,
        &output,
        &u_output_size);
    *output = '\0';

    if (nconv == static_cast<size_t>(-1)) {
      return false;
    } else {
      return true;
    }
  }

  bool UTF8ToGBK(const char *input, char *output, int output_size) {
    size_t input_size = strlen(input);
    size_t u_output_size = output_size;

    size_t nconv = iconv(
        iconv_utf8_to_gbk_,
        const_interoperable<char **>(&input),
        &input_size,
        &output,
        &u_output_size);
    *output = '\0';

    if (nconv == static_cast<size_t>(-1)) {
      return false;
    } else {
      return true;
    }
  }

 private:
  iconv_t iconv_gbk_to_utf8_;
  iconv_t iconv_utf8_to_gbk_;
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
