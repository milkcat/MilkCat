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
// log.h --- Created at 2014-05-06
//

#ifndef SRC_UTILS_LOG_H_
#define SRC_UTILS_LOG_H_

#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <string>

namespace milkcat {

class LogUtil {
 public:
  LogUtil();

  // The << operators
  LogUtil &operator<<(int val) { stream_ << val; return *this; }
  LogUtil &operator<<(unsigned int val) { stream_ << val; return *this; }
  LogUtil &operator<<(short val) { stream_ << val; return *this; }
  LogUtil &operator<<(unsigned short val) { stream_ << val; return *this; }
  LogUtil &operator<<(long val) { stream_ << val; return *this; }
  LogUtil &operator<<(unsigned long val) { stream_ << val; return *this; }
  LogUtil &operator<<(float val) { stream_ << val; return *this; }
  LogUtil &operator<<(double val) { stream_ << val; return *this; }
  LogUtil &operator<<(const char *val) {
    stream_ << val;
    return *this;
  }
  LogUtil &operator<<(const std::string *val) {
    stream_ << val;
    return *this;
  }
  LogUtil &operator<<(char val) { stream_ << val; return *this; }

  // Get the string value of the input data
  std::string GetString() const;

 private:
  std::ostringstream stream_;
};

}  // namespace milkcat

#endif  // SRC_UTILS_LOG_H_