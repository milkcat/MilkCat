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
// nekoneko.h --- Created at 2014-03-18
//

#ifndef SRC_NEKONEKO_NEKONEKO_H_
#define SRC_NEKONEKO_NEKONEKO_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

typedef struct nekoneko_result_t nekoneko_result_t;

// Extracts new words from text file specified by corpus_path.
// vocabulary_word_file is the file contains the knowned vocabulary word,
// log_func is called when some message would output, set NULL to ignore,
// progress_func is the function to show progress, set NULL to ignore.
// On success, returns the word extracted from file as a 
// nekoneko_result_t pointer, use nekoneko_result_get_XXX to get the results.
// On failed, returns NULL
nekoneko_result_t *nekoneko_extract(
    const char *corpus_path,
    const char *vocabulary_word_file,
    void (* log_func)(const char *msg),
    void (* progress_func)(int64_t bytes_processed,
                           int64_t file_size,
                           int64_t bytes_per_second));

// Gets the number of words in result
int nekoneko_result_size(nekoneko_result_t *result);

// Gets the word in the position of result specified by pos
const char *nekoneko_result_get_word_at(nekoneko_result_t *result, int pos);

// Gets the word's weight in the position of result specified by pos. The weight
// is larger when its corresponding word is more likely to be a real word
double nekoneko_result_get_weight_at(nekoneko_result_t *result, int pos);

// Destroys the result returned by nekoneko_extract
void nekoneko_result_destroy(nekoneko_result_t *result);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // SRC_NEKONEKO_NEKONEKO_H_
