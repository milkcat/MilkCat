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
// milkcat.h --- Created at 2013-09-03
//



#ifndef SRC_MILKCAT_MILKCAT_H_
#define SRC_MILKCAT_MILKCAT_H_

#include <stdbool.h>

#ifdef _WIN32
#ifdef MILKCAT_EXPORTS
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
#endif
#else
#define EXPORT_API
#endif

typedef struct milkcat_t milkcat_t;
typedef struct milkcat_model_t milkcat_model_t;
typedef struct milkcat_cursor_t milkcat_cursor_t;

#ifdef __cplusplus
extern "C" {
#endif

enum {
  TOKENIZER_NORMAL = 0x00000001,

  SEGMENTER_CRF = 0x00000010,
  SEGMENTER_UNIGRAM = 0x00000020,
  SEGMENTER_BIGRAM = 0x00000030,
  SEGMENTER_MIXED = 0x00000040,

  POSTAGGER_HMM = 0x00001000,
  POSTAGGER_CRF = 0x00002000,
  POSTAGGER_MIXED = 0x00003000
};

enum {
  DEFAULT_ANALYZER = TOKENIZER_NORMAL | SEGMENTER_MIXED | POSTAGGER_MIXED,
  DEFAULT_SEGMENTER = TOKENIZER_NORMAL | SEGMENTER_MIXED,

  CRF_SEGMENTER = TOKENIZER_NORMAL | SEGMENTER_CRF,
  CRF_ANALYZER = TOKENIZER_NORMAL | SEGMENTER_CRF | POSTAGGER_CRF,

  BIGRAM_SEGMENTER = TOKENIZER_NORMAL | SEGMENTER_BIGRAM,
  UNIGRAM_SEGMENTER = TOKENIZER_NORMAL | SEGMENTER_BIGRAM
};

// Word types
#define MC_CHINESE_WORD 0
#define MC_ENGLISH_WORD 1
#define MC_NUMBER 2
#define MC_SYMBOL 3
#define MC_PUNCTION 4
#define MC_OTHER 5

// MilkCat cursor return state
#define MC_OK 1
#define MC_NONE 0

typedef struct {
  const char *word;
  const char *part_of_speech_tag;
  int word_type;
} milkcat_item_t;


EXPORT_API milkcat_t *milkcat_new(milkcat_model_t *model, int analyzer_type);

// Delete the MilkCat Process Instance and release its resources
EXPORT_API void milkcat_destroy(milkcat_t *m);

// Start to Process a text
EXPORT_API void milkcat_analyze(milkcat_t *m, 
                                milkcat_cursor_t *cursor,
                                const char *text);

EXPORT_API milkcat_cursor_t *milkcat_cursor_new();

EXPORT_API void milkcat_cursor_destroy(milkcat_cursor_t *cursor);

// Goto the next word in the text, if end of the text reached return 0 else
// return 1
EXPORT_API int milkcat_cursor_get_next(milkcat_cursor_t *c,
                                       milkcat_item_t *next_item);

EXPORT_API milkcat_model_t *milkcat_model_new(const char *model_path);

EXPORT_API void milkcat_model_destroy(milkcat_model_t *model);

EXPORT_API void milkcat_model_set_userdict(milkcat_model_t *model,
                                           const char *path);

// Get the error message if an error occurred
EXPORT_API const char *milkcat_last_error();

#ifdef __cplusplus
}
#endif

#endif  // SRC_MILKCAT_MILKCAT_H_
