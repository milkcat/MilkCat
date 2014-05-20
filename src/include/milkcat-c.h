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
// milkcat-c.h --- Created at 2014-05-19
//

// The C API header file for MilkCat. It only supports the Parser API now.

#ifndef MILKCAT_C_H_
#define MILKCAT_C_H_

#ifdef __cplusplus
#define EXPORT_API extern "C"
#else
#define EXPORT_API
#endif

#include <stdint.h>

// This enum represents the type or the algorithm of Parser. It could be
// kDefault which indicates using the default algorithm for segmentation and
// part-of-speech tagging. Meanwhile, it could also be 
//   kTextTokenizer | kBigramSegmenter | kHmmTagger
// which indicates using bigram segmenter for segmentation, using HMM model
// for part-of-speech tagging.
enum ParserType {
  // Tokenizer type
  kTextTokenizer = 0,

  // Segmenter type
  kMixedSegmenter = 0x00000000,
  kCrfSegmenter = 0x00000010,
  kUnigramSegmenter = 0x00000020,
  kBigramSegmenter = 0x00000030,

  // Part-of-speech tagger type
  kMixedTagger = 0x00000000,
  kHmmTagger = 0x00001000,
  kCrfTagger = 0x00002000,
  kNoTagger = 0x000ff000,

  kDefault = 0,
  kSegmenter = kNoTagger
};

// The type of word. If the word is a Chinese word, English word or it's a
// number or ...
enum WordType {
  kChineseWord = 0,
  kEnglishWord = 1,
  kNumber = 2,
  kSymbol = 3,
  kPunction = 4,
  kOther = 5
};


// The type for Model
typedef void * milkcat_model_t;

// The type for Parser
typedef void * milkcat_parser_t;

// The type for Parser::Iterator
typedef void * milkcat_parser_iterator_t;

// milkcat_parser_create creates an parser with parameters specified by model
// and type. 
// model is the model class for the parser, NULL is to use
// the default internal model created by parser itself.
// type is the the algorithm used by the parser. 
// On success, return zero and store the handle of the created parser in the
// location referenced by parser. On failed, return non-zero value and the
// error message could be obtained by milkcat_last_error()
EXPORT_API int
milkcat_parser_create(milkcat_parser_t *parser,
                      milkcat_model_t model,
                      int type);

// Destroy the parser
EXPORT_API void milkcat_parser_destroy(milkcat_parser_t parser);

// Parses the text and returns an iterator of each words with the part-of-speech
// tags. The iterator should be released by milkcat_parser_release_iterator()
EXPORT_API milkcat_parser_iterator_t
milkcat_parser_parse(milkcat_parser_t parser, const char *text);

// Gets current word in the iterator. The string it pointed to is vaild until
// the milkcat_parser_iterator_next() call.
EXPORT_API const char *
milkcat_parser_iterator_word(milkcat_parser_iterator_t it);

// Gets current part_of_speech tag in the iterator. The string it pointed to is
// vaild until the milkcat_parser_iterator_next() is called.
EXPORT_API const char *
milkcat_parser_iterator_tag(milkcat_parser_iterator_t it);

// Returns the type of the current word in iterator. The returned value is one
// of the values in WordType.
EXPORT_API int
milkcat_parser_iterator_type(milkcat_parser_iterator_t it);

// Returns a value > 0 if the the iterator has an next item. Returns 0 when the
// end of iterator is reached.
EXPORT_API int
milkcat_parser_iterator_has_next(milkcat_parser_iterator_t it);

// Moves to the next item of the iterator.
EXPORT_API void milkcat_parser_iterator_next(milkcat_parser_iterator_t it);

// Release the iterator returned by milkcat_parser_parse()
EXPORT_API void milkcat_parser_release_iterator(milkcat_parser_t parser,
                                                milkcat_parser_iterator_t it);

// milkcat_model_new creates the model instance which is used by parser.
// model_dir is the directory of the model data, NULL is to use the default
// model dir. 
// On success, return zero and store the handle of the created model in the
// location referenced by model. On failed, return a non-zero value, and error
// message could be obtained via milkcat_last_error()
EXPORT_API int
milkcat_model_create(milkcat_model_t *model, const char *model_dir);

// Destroys the model.
EXPORT_API void milkcat_model_destroy(milkcat_model_t model);

// Gets the last error message
EXPORT_API const char *milkcat_last_error();

#endif  // MILKCAT_C_H_
