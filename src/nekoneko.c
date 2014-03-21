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
// nekoneko.c --- Created at 2014-03-21
//

#include <stdio.h>
#include "nekoneko/nekoneko.h"

void display_progress(int64_t bytes_processed,
                      int64_t file_size,
                      int64_t bytes_per_second) {
  printf("\rprogress %dMB/%dMB -- %2.1f%% %.3fMB/s",
         (int)(bytes_processed / (1024 * 1024)),
         (int)(file_size / (1024 * 1024)),
         100.0 * bytes_processed / file_size,
         bytes_per_second / (double)(1024 * 1024));
  if (bytes_processed == file_size) puts("");
  fflush(stdout);
}

int main(int argc, char **argv) {
  nekoneko_result_t *r = NULL;
  int i;

  r = nekoneko_extract(argv[1], NULL, puts, display_progress);
  for (i = 0; i < nekoneko_result_size(r); ++i) {
    printf("%s %lf\n",
           nekoneko_result_get_word_at(r, i),
           nekoneko_result_get_weight_at(r, i));
  }

  return 0;
}

