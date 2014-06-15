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
// document.cc --- Created at 2014-03-26
//

#include "keyphrase/document.h"

#include "common/trie_tree.h"
#include "include/milkcat.h"
#include "utils/utils.h"

namespace milkcat {

Document::Document(const TrieTree *stopword_index):
    stopword_index_(stopword_index) {
}

void Document::Add(const char *word_str, int word_type) {
  int word = string_table_.GetOrInsertId(word_str);
  document_.push_back(word);
  
  bool is_stopword = false;
  if (word_type != Parser::kChineseWord && word_type != Parser::kEnglishWord) {
    is_stopword = true;
  } else if (stopword_index_->Search(word_str) >= 0) {
    is_stopword = true;
  }

  // If a new word arraived
  if (word == inverted_index_.size()) {
    inverted_index_.push_back(std::vector<int>());
    is_stopword_.push_back(is_stopword);
  }

  inverted_index_[word].push_back(document_.size() - 1);
}

void Document::Clear() {
  string_table_.Clear();
  document_.clear();
  is_stopword_.clear();
  inverted_index_.clear();
}

}  // namespace milkcat