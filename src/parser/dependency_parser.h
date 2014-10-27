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
// dependency_parser.h --- Created at 2013-08-12
//


#ifndef SRC_PERSER_DEPENDENCY_PARSER_H_
#define SRC_PERSER_DEPENDENCY_PARSER_H_

namespace milkcat {

class TermInstance;
class PartOfSpeechTagInstance;
class DependencyInstance;
class Status;
class ReadableFile;

// Base class for dependency parser
class DependencyParser {
 public:
  class Node;
  class FeatureTemplate;
  class State;

  virtual ~DependencyParser();

  virtual void Parse(
    DependencyInstance *dependency_instance,
    const TermInstance *term_instance,
    const PartOfSpeechTagInstance *part_of_speech_tag_instance) = 0;

  // Load a dependency tree from training corpus, Stores it into term_instance,
  // tag_instance and dependency_instance
  static void LoadDependencyTreeInstance(
      ReadableFile *fd,
      TermInstance *term_instance,
      PartOfSpeechTagInstance *tag_instance,
      DependencyInstance *dependency_instance,
      Status *status);

  // Gets the Unlabeled Attachment Score (UAS) and Labeled Attachment Score
  // (LAS) of the `parser` in `test_corpus`. Stores them into `UAS` and `LAS`
  static void Test(
      const char *test_corpus,
      DependencyParser *parser,
      double *LAS,
      double *UAS,
      Status *status);
};

inline DependencyParser::~DependencyParser() {
}

}  // namespace milkcat

#endif  // SRC_PERSER_DEPENDENCY_PARSER_H_
