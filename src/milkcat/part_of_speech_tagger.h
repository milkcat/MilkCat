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
// part_of_speech_tagger.cc --- Created at 2013-11-24
//

#ifndef SRC_MILKCAT_PART_OF_SPEECH_TAGGER_H_
#define SRC_MILKCAT_PART_OF_SPEECH_TAGGER_H_

namespace milkcat {

class PartOfSpeechTagInstance;
class TermInstance;

// The base class for part-of-speech tagger
class PartOfSpeechTagger {
 public:
  virtual ~PartOfSpeechTagger() = 0;

  // Tag the TermInstance and put the result to PartOfSpeechTagInstance
  virtual void Tag(PartOfSpeechTagInstance *part_of_speech_tag_instance,
                   TermInstance *term_instance) = 0;
};

inline PartOfSpeechTagger::~PartOfSpeechTagger() {}

}  // namespace milkcat

#endif  // SRC_MILKCAT_PART_OF_SPEECH_TAGGER_H_
