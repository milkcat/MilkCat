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
#include <string.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

inline const char *_filename(const char *path) {
  int len = strlen(path);
  const char *p = path + len;

  while (*(p - 1) != '/' && *(p - 1) != '\\' && p != path) p--;
  return p;
}

#ifdef ENABLE_LOG
#define _LOG(x) \
        puts((LogUtil() << "[" << _filename(__FILE__) << __LINE__ << "] " \
                        << x).GetString().c_str())
#else
#define _LOG(x)
#endif

#ifdef ENABLE_LOG
#define LOG_IF(cond, x) if (cond) \
        puts((LogUtil() << "[" << _filename(__FILE__) << __LINE__ << "] " \
                        << x).GetString().c_str())
#else
#define LOG_IF(...)
#endif

// If we have the c++11 compiler
#if __cplusplus >= 201103L

namespace milkcat {
namespace logging {
template<typename T>
inline void print_log(const T &t) {
  std::cout << t << std::endl;
}
template<typename T, typename... Args>
inline void print_log(const T &first, Args &&...rest) {
  std::cout << first;
  print_log(rest...);
}
}  // namespace log
}  // namespace milkcat

#ifdef DEBUG
#define LOG(...) do { std::cout << "[" << _filename(__FILE__) << ":" \
        << __LINE__ << "] "; milkcat::logging::print_log(__VA_ARGS__); } \
        while (0);
#else
#define LOG(...)
#endif

#else

#ifdef DEBUG
#define LOG(...) do { std::cout << "[" << _filename(__FILE__) << ":" \
        << __LINE__ << "] "; puts("LOG needs c++11 support!"); } \
        while (0);
#else
#define LOG(...)
#endif

#endif  // __cplusplus >= 201103L


#ifndef NOASSERT
#define ASSERT(cond, message) \
        if (!(cond)) { \
          fprintf(stderr,  \
                  "[%s:%d] ASSERT failed: ", \
                  _filename(__FILE__), \
                  __LINE__); \
          fputs(message, stderr); \
          fputs("\n", stderr); \
          exit(1); \
        }
#else
#define ASSERT(cond, message)
#endif  // NOASSERT

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
  LogUtil &operator<<(const std::string &val) {
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