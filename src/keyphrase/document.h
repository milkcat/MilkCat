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
// document.h --- Created at 2014-03-26
//

#ifndef SRC_KEYPHRASE_DOCUMENT_H_
#define SRC_KEYPHRASE_DOCUMENT_H_

#include <assert.h>
#include <set>
#include <vector>
#include "keyphrase/string_table.h"
#include "utils/utils.h"

namespace milkcat {

class TrieTree;

// This class represent a document, it stores each words' index and a vector
// indicates which word is a stopword
class Document {
 public:
  Document(const TrieTree *stopword_index);

  // Add a word and its type into the document
  void Add(const char *word_str, int word_type);

  // Clear all the words of the document
  void Clear();

  // Returns the size of document (how many words it contains)
  int size() const { return document_.size(); }

  // Gets the id of term at the idx of document
  int word(int index) const {
    assert(index < document_.size());
    return document_[index]; 
  }

  // Returns true if the term specified by word is a stopword
  int is_stopword(int word) const { 
    assert(word < string_table_.size());
    return is_stopword_[word]; 
  }

  // Gets the inversed index of word
  const std::vector<int> &word_index(int word) const {
    return inverted_index_[word];
  }

  // Number of  size of the document
  int words_size() const { return string_table_.size(); }

  // Gets the term frequency of the term specified by word. Term frequency
  // for stopwords is 0
  int tf(int word) const {
    return inverted_index_[word].size();
  }

  const char *word_str(int word) const { 
    return string_table_.GetStringById(word);
  }

 private:
  StringTable string_table_;
  std::vector<int> document_;
  std::vector<bool> is_stopword_;
  std::vector<std::vector<int> > inverted_index_;
  const TrieTree *stopword_index_;

  DISALLOW_COPY_AND_ASSIGN(Document);
};

}  // namespace milkcat


#endif  // SRC_KEYPHRASE_DOCUMENT_H_