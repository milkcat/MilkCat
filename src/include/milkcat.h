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

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus

namespace milkcat {

class Parser;

// ---------------------------- Model ----------------------------------------

class Model {
 public:
  class Impl;

  ~Model();
  
  // Create the model for further use. model_dir is the path of the model data
  // dir, NULL is to use the default data dir.
  static Model *New(const char *model_dir = NULL);

  // Set the user dictionary for segmenter. On success, return true. On failed,
  // return false;
  bool SetUserDictionary(const char *userdict_path);

  // Get the instance of the implementation class
  Impl *impl() { return impl_; }

 private:
  Model();
  Impl *impl_;
};

// ---------------------------- Parser ---------------------------------------

// Parser is the word segmenter and part-of-speech tagger class. Use 'Parse' to
// parse the text and it returns an Parser::Iterator to iterate each word and
// its part-of-speech tag. After iterating, you should use Parser::Release to
// release the iterator
class Parser {
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

  ~Parser();

  // Create the parser. 
  static Parser *New();
  static Parser *New(const Options &options);
  static Parser *New(const Options &options, Model *model);

  // Parses the text and stores the result into the iterator
  void Parse(const char *text, Iterator *iterator);

  // Get the instance of the implementation class
  Impl *impl() { return impl_; }

 private:
  Parser();
  Impl *impl_;
};

// The options for the parser
class Parser::Options {
 public:
  Options();

  // The type of segmenter, part-of-speech tagger and dependency parser
  // Default is MixedSegmenter, MixedPOSTagger and NoDependencyParser
  void UseMixedSegmenter();
  void UseCrfSegmenter();
  void UseUnigramSegmenter();
  void UseBigramSegmenter();

  void UseMixedPOSTagger();
  void UseHmmPOSTagger();
  void UseCrfPOSTagger();
  void NoPOSTagger();

  void UseArcEagerDependencyParser();
  void NoDependencyParser();

  // Get the type value of current setting
  int TypeValue() const;

 private:
  int segmenter_type_;
  int tagger_type_;
  int parser_type_;
};

class Parser::Iterator {
 public:
  class Impl;

  Iterator();
  ~Iterator();

  // Returns true if it reaches the end of text
  bool End();

  // Go to the next element
  void Next();

  // Get the string of current word
  const char *word() const;

  // Get the string of current part-of-speech tag
  const char *part_of_speech_tag() const;

  // Get the head of current node (in dependency tree)
  int head_node() const;

  // Get the depengency type of current node
  const char *dependency_type() const;

  // Get the type of current word (chinese word or english word or ...)
  int type() const;

  // Get the instance of the implementation class
  Impl *impl() { return impl_; }

 private:
  Impl *impl_;
};

// Get the information of last error
const char *LastError();

}  // namespace milkcat

#endif  // __cplusplus

#endif  // MILKCAT_H_
