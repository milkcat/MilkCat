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
// trie_tree.h --- Created at 2013-12-05
//

#ifndef SRC_COMMON_TRIE_TREE_H_
#define SRC_COMMON_TRIE_TREE_H_

#include <map>
#include <string>
#include "common/darts.h"
#include "utils/utils.h"

namespace milkcat {

class TrieTree {
 public:
  static const int kExist = -1;
  static const int kNone = -2;

  virtual ~TrieTree() = 0;

  // Search a trie tree if text exists return its value
  // else the return value is < 0
  virtual int Search(const char *text) const = 0;
  virtual int Search(const char *text, int len) const = 0;

  // Traverse a trie tree the node is the last state and will changed during
  // traversing for the root node the node = 0, return value > 0 if text exists
  // returns kExist if something with text as its prefix exists buf text itself
  // doesn't exist return kNone it text doesn't exist.
  virtual int Traverse(const char *text, size_t *node) const = 0;
};

inline TrieTree::~TrieTree() {}

class DoubleArrayTrieTree: public TrieTree {
 public:
  static DoubleArrayTrieTree *New(const char *file_path, Status *status);

  // Create the double array from the map data. The key of map is the
  // word itself, and the value of map is the id of word
  static DoubleArrayTrieTree *NewFromMap(
    const std::map<std::string, int> &src_map);
  DoubleArrayTrieTree() {}

  int Search(const char *text) const;
  int Search(const char *text, int len) const;
  int Traverse(const char *text, size_t *node) const;

 private:
  Darts::DoubleArray double_array_;

  DISALLOW_COPY_AND_ASSIGN(DoubleArrayTrieTree);
};

}  // namespace milkcat

#endif  // SRC_COMMON_TRIE_TREE_H_
