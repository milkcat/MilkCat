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
// utils.h --- Created at 2013-08-10
// util.h --- Created at 2015-01-28
//

#ifndef SRC_UTIL_UTIL_H_
#define SRC_UTIL_UTIL_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "util/status.h"

namespace milkcat {

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz);

char *trim(char *str);
const char *_filename(const char *path);

// 64-bit version of ftell
int64_t ftell64(FILE *fd);

char *strtok_r(char *s, const char *delim, char **last);

template<class T>
class const_interoperable {}; 

template<class T>
class const_interoperable<T **> { 
 public: 
  const_interoperable(T **t) : t(t) {}
  const_interoperable(const T **t) : t(const_cast<T **>(t)) {}

  operator T**() const { return t; }
  operator const T**() const { return const_cast<const T **>(t); }
 private:
  T **t;
};

}  // namespace milkcat

#ifdef DEBUG
#define LOG(...)  printf(__VA_ARGS__)
#else 
#define LOG(...)
#endif 

#define MC_ERROR(message) \
        do { \
          fprintf(stderr,  \
                  "[%s:%d] ERROR: ", \
                  _filename(__FILE__), \
                  __LINE__); \
          fputs(message, stderr); \
          fputs("\n", stderr); \
          exit(1); \
        } while (0);


#ifndef NOASSERT
#define MC_ASSERT(cond, message) \
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
#define MC_ASSERT(cond, message)
#endif  // NOASSERT

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
        TypeName(const TypeName&); \
        void operator=(const TypeName&)

#endif  // SRC_UTIL_UTIL_H_
