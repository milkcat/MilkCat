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

#include <stdint.h>

#ifdef _WIN32
#ifdef MILKCAT_EXPORTS
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
#endif
#else
#define EXPORT_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------ MilkCat start ~!! ---------------------------------

typedef struct milkcat_t milkcat_t;
typedef struct milkcat_model_t milkcat_model_t;
typedef struct milkcat_cursor_t milkcat_cursor_t;

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


milkcat_t *milkcat_new(milkcat_model_t *model, int analyzer_type);

// Delete the MilkCat Process Instance and release its resources
void milkcat_destroy(milkcat_t *analyzer);

// Start to analyze a text
milkcat_item_t *milkcat_analyze(milkcat_t *analyzer, const char *text);

milkcat_model_t *milkcat_model_new(const char *model_path);

void milkcat_model_destroy(milkcat_model_t *model);

void milkcat_model_set_userdict(milkcat_model_t *model, const char *path);

// Get the error message if an error occurred
const char *milkcat_last_error();

// -------------------------- MilkCat end ~!! ---------------------------------

// ------------------------ Nekoneko start ~!! --------------------------------

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

// -------------------------- Nekoneko end ~!! --------------------------------

// ------------------------ Keyphrase start ~!! -------------------------------

typedef struct {
  const char *keyphrase;
  double weight;
} keyphrase_item_t;

typedef struct milkcat_keyphrase_t milkcat_keyphrase_t;

#define MILKCAT_KEYPHRASE_DEFAULT 0

milkcat_keyphrase_t *
milkcat_keyphrase_new(milkcat_model_t *model, int method);

keyphrase_item_t *
milkcat_keyphrase_extract(milkcat_keyphrase_t *extractor, const char *text);

void milkcat_keyphrase_destroy(milkcat_keyphrase_t *extractor);

// -------------------------- Keyphrase end ~!! -------------------------------

#ifdef __cplusplus
}
#endif

#endif  // SRC_MILKCAT_MILKCAT_H_
