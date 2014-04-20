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
// phrase.h --- Created at 2014-04-04
//

#ifndef SRC_KEYPHRASE_PHRASE_H_
#define SRC_KEYPHRASE_PHRASE_H_

#include <assert.h>
#include <string>
#include <vector>
#include "keyphrase/document.h"

namespace milkcat {

// A phrase in document
class Phrase {
 public:
  Phrase(): document_(NULL), tf_(0.0), weight_(0.0) {
  }

  void set_document(const Document *document) {
    document_ = document;
  }

  void set_words(const std::vector<int> &words) {
    words_ = words;
  }

  // The term frequency value
  void set_tf(double tf) { tf_ = tf; }
  double tf() const { return tf_; }

  // Final weight for the phrase
  void set_weight(double weight) { weight_ = weight; }
  double weight() const { return weight_; }

  // Gets the string of this phrase
  const char *PhraseString() {
    if (phrase_string_.size() == 0) {
      for (int i = 0; i < words_.size(); ++i)
        phrase_string_ += document_->word_str(words_[i]);
    }

    return phrase_string_.c_str();
  }

  // Word's string at index
  const char *WordString(int index) const {
    assert(index < word_count());
    
    int word = words_[index];
    return document_->word_str(word);
  }

  // Number of words in this phrase
  int word_count() const { return words_.size(); }

 private:
  const Document *document_;
  double tf_;
  double weight_;
  std::vector<int> words_;
  std::string phrase_string_;
};

}  // namespace milkcat

#endif  // SRC_KEYPHRASE_PHRASE_H_