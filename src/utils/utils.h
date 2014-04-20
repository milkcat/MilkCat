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
//



#ifndef SRC_UTILS_UTILS_H_
#define SRC_UTILS_UTILS_H_

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "utils/status.h"

#if defined(HAVE_UNORDERED_MAP)
#include <unordered_map>
#elif defined(HAVE_TR1_UNORDERED_MAP)
#include <tr1/unordered_map>
#endif

namespace milkcat {
namespace utils {


size_t strlcpy(char *dst, const char *src, size_t siz);
char *trim(char *str);

// Sleep for seconds
void Sleep(double seconds);

// Get number of processors/cores in current machine
int HardwareConcurrency();

#if defined(HAVE_UNORDERED_MAP)
using std::unordered_map;
#elif defined(HAVE_TR1_UNORDERED_MAP)
using std::tr1::unordered_map;
#endif

}  // namespace utils
}  // namespace milkcat


#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
            TypeName(const TypeName&); \
            void operator=(const TypeName&)

inline const char *_filename(const char *path) {
  int len = strlen(path);
  const char *p = path + len;

  while (*(p - 1) != '/' && *(p - 1) != '\\' && p != path) p--;
  return p;
}

#ifdef ENABLE_LOG
#define LOG(format, ...) \
        fprintf(stderr, "[%s:%d] " format "\n", _filename(__FILE__), \
                __LINE__, __VA_ARGS__)
#else
#define LOG(...)
#endif

#ifdef ENABLE_LOG
#define LOG_IF(cond, format, ...) if (cond) \
        fprintf(stderr, "[%s:%d] " format "\n", _filename(__FILE__), \
                __LINE__, __VA_ARGS__)
#else
#define LOG_IF(...)
#endif


#endif  // SRC_UTILS_UTILS_H_
