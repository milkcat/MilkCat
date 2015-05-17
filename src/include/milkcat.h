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

#ifndef MILKCAT_H_
#define MILKCAT_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32

#ifdef MILKCAT_EXPORTS
#define MILKCAT_API __declspec(dllexport)
#else
#define MILKCAT_API __declspec(dllimport)
#endif  // MILKCAT_EXPORTS

#else  // _WIN32

#define MILKCAT_API

#endif  // _WIN32

#ifdef __cplusplus

namespace milkcat {

class MILKCAT_API noncopyable {
 protected:
  noncopyable() {}
  ~noncopyable() {}
 private:
  noncopyable(const noncopyable &);
  const noncopyable &operator=(const noncopyable &);
};

// ---------------------------- Parser ---------------------------------------

// Parser is the main class in MilkCat. Use `Predict` to predict the structure
// (including words, part-of-speech tags and dependency tree) of input text.
class MILKCAT_API Parser: public noncopyable {
 public:
  class Impl;
  class Iterator;
  class Options;

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

  Parser();
  ~Parser();

  // Creates the Parser instance by Impl, ONLY for internal usage
  Parser(Impl *impl);

  // Creates the parser with `options`.
  explicit Parser(const Options &options);

  // Returns true when successfully initialized.
  bool ok() { return impl_ != NULL; }

  // Parses the text and stores the result into the iterator
  void Predict(Iterator *iterator, const char *text);

  // Get the instance of the implementation class, ONLY for internal usage
  Impl *impl() const { return impl_; }

 private:
  Impl *impl_;
};

// The options for the parser
class MILKCAT_API Parser::Options {
 public:
  class Impl;

  Options();
  Options(const Options &options);
  ~Options();

  Options &operator=(const Options &options);

  // Use GBK or UTF-8 encoding
  void UseGBK();
  void UseUTF8();

  // The type of segmenter, part-of-speech tagger and dependency parser
  // Default is MixedSegmenter, MixedPOSTagger and NoDependencyParser
  void UseMixedSegmenter();
  void UseCRFSegmenter();
  void UseUnigramSegmenter();
  void UseBigramSegmenter();

  void UseMixedPOSTagger();
  void UseHMMPOSTagger();
  void UseCRFPOSTagger();
  void NoPOSTagger();

  void UseYamadaParser();
  void UseBeamYamadaParser();
  void NoDependencyParser();

  // Sets the path of model data, the `model_path` corresponds a directory
  // in filesystem
  void SetModelPath(const char *model_path);

  // Sets the user directory for word segmenter.
  void SetUserDictionary(const char *userdict_path);

  // Get the instance of the implementation class
  Impl *impl() const { return impl_; }

 private:
  Impl *impl_;
};

//
// ParserPool is the class that can create multiple Parser instances which
// shared the same model data. It mainly used in multi-thread programs since
// Parser is not thread safe and users should create the Parser instance for
// each threads.
// 
// Usage:
//   milkcat::ParserPool parser_pool;
//   if (parser_pool.ok()) {
//     milkcat::Parser *parser1 = parser_pool.NewParser();
//     milkcat::Parser *parser2 = parser_pool.NewParser();
//     ...
//     parser_pool.ReleaseAll();
//   } else {
//     puts(milkcat::LastError())
//   }
//
class MILKCAT_API ParserPool {
 public:
  ParserPool();
  ~ParserPool();

  // Creates the parser with `options`. The usage of options is just the same
  // as `Parser::Parser()`
  explicit ParserPool(const Parser::Options &options);

  // Creates the Parser instance. When ParserPool::ok() is false, it retuens
  // a NULL pointer. And use ParserPool::ReleaseAll() to release the instances
  // it creates. Users SHOULD NOT delete the instance by himself, or the
  // program would be crashed.
  Parser *NewParser();

  // Releases all the Parser instances it creates
  void ReleaseAll();

  // Returns true when successfully initialized.
  bool ok() { return impl_ != NULL; }

 private:
  class Impl;
  Impl *impl_;
};

// Iterator for the prediction of text from Parser
class MILKCAT_API Parser::Iterator: public noncopyable {
 public:
  class Impl;

  Iterator();
  ~Iterator();

  // Go to the next element, returns true when have next word, returns false
  // when reaches the end of text.
  bool Next();

  // Get the string of current word
  const char *word() const;

  // Get the string of current part-of-speech tag
  const char *part_of_speech_tag() const;

  // Get the head of current node (in dependency tree)
  int head() const;

  // Get the depengency lebel of current node
  const char *dependency_label() const;

  // Get the type of current word (chinese word or english word or ...)
  int type() const;

  // Returns true if this word is the begin of sentence (BOS)
  bool is_begin_of_sentence() const;

  // Get the instance of the implementation class
  Impl *impl() { return impl_; }

 private:
  Impl *impl_;
};

// Get the information of last error
MILKCAT_API const char *LastError();

}  // namespace milkcat

#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct milkcat_parser_t milkcat_parser_t;

typedef struct milkcat_parseriter_internal_t milkcat_parseriter_internal_t;
typedef struct milkcat_parseriterator_t {
  const char *word;
  const char *part_of_speech_tag;
  int head;
  const char *dependency_label;
  bool is_begin_of_sentence;
  milkcat_parseriter_internal_t *it;
} milkcat_parseriterator_t;

#define MC_SEGMENTER_BIGRAM 0
#define MC_SEGMENTER_CRF 1
#define MC_SEGMENTER_MIXED 2

#define MC_POSTAGGER_MIXED 0
#define MC_POSTAGGER_CRF 1
#define MC_POSTAGGER_HMM 2
#define MC_POSTAGGER_NONE 3

#define MC_DEPPARSER_YAMADA 0
#define MC_DEPPARSER_BEAMYAMADA 1
#define MC_DEPPARSER_NONE 2

typedef struct milkcat_parseroptions_t {
  int word_segmenter;
  int part_of_speech_tagger;
  int dependency_parser;
  const char *user_dictionary_path;
  const char *model_path;
} milkcat_parseroptions_t;

MILKCAT_API void milkcat_parseroptions_init(
    milkcat_parseroptions_t *parseropt);
MILKCAT_API milkcat_parser_t *milkcat_parser_new(
    milkcat_parseroptions_t *parseropt);
MILKCAT_API void milkcat_parser_destroy(milkcat_parser_t *model);
MILKCAT_API void milkcat_parser_predict(
    milkcat_parser_t *parser,
    milkcat_parseriterator_t *parseriter,
    const char *text);

MILKCAT_API milkcat_parseriterator_t *milkcat_parseriterator_new(void);
MILKCAT_API void milkcat_parseriterator_destroy(
    milkcat_parseriterator_t *parseriter);
MILKCAT_API bool milkcat_parseriterator_next(
    milkcat_parseriterator_t *parseriter);

MILKCAT_API const char *milkcat_last_error();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // MILKCAT_H_
