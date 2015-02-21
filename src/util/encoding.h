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
// encoding.h --- Created at 2015-02-20
//

#ifndef SRC_UTIL_ENCODING_H_
#define SRC_UTIL_ENCODING_H_

namespace milkcat {

// Converts strings between different encodings
class Encoding {
 public:
  class Impl;

  Encoding();
  ~Encoding();

  // Converts GBK string to UTF-8, returns true if successed.
  bool GBKToUTF8(const char *input, char *output, int output_size);

  // Converts UTF8 string to GBK, returns true if successed.
  bool UTF8ToGBK(const char *input, char *output, int output_size);

 private:
  Impl *impl_;
};

}

#endif  // SRC_UTIL_ENCODING_H_