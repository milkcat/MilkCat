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
// string_builder.h --- Created at 2014-06-03
//

#include <stdio.h>
#include "utils/utils.h"

namespace milkcat {

// StringBuilder builds a string with a limited size. It writes to the buffer
// directly instead of use its own buffer.
class StringBuilder {
 public:
  StringBuilder(char *buffer, int capability): 
      buffer_(buffer), capability_(capability), size_(0) {
  }

  // Append functions
  StringBuilder &operator <<(const char *str) {
    // LOG("Append string: " << str);
    int len = strlcpy(buffer_ + size_, str, capability_ - size_);
    size_ += len;
    return *this;
  }
  StringBuilder &operator <<(char ch) {
    if (capability_ > size_ + 1) {
      buffer_[size_++] = ch;
      buffer_[size_] = '\0';
    }
    return *this;
  }
  StringBuilder &operator <<(int val) {
    int len = sprintf(buffer_ + size_, "%d", val);
    size_ += len;
    return *this;
  }

 private:
  char *buffer_;
  int capability_;
  int size_;
};

}  // namespace milkcat