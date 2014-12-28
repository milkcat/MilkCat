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
// reimu_trie.h --- Created at 2014-10-16
// Reimu x Marisa :P
//

#ifndef REIMU_TRIE_H_
#define REIMU_TRIE_H_

namespace milkcat {

// RemmuTrie is a reimplementation of the double-array trie algorithm of
// cedar (http://www.tkl.iis.u-tokyo.ac.jp/~ynaga/cedar/)
class ReimuTrie {
 public:
  typedef int int32;
  typedef unsigned char uint8;

  enum {
    kNoValue = -1,
    kNoPath = -2
  };

  ReimuTrie();
  ~ReimuTrie();

  // Open a ReimuTrie saved file. On success, returns an instance of ReimuTrie.
  // On failed, returns NULL
  static ReimuTrie *Open(const char *filename);

  // Gets the corresponded value for `key`, if `key` does not exist, returns
  // `default_value`
  int32 Get(const char *key, int32 default_value);

  // Traverse ReimuTrie from `*from` and gets the value of `key`, then sets
  // `from` to the latest position in the trie. If the path didn't exist,
  // just returns false. If the path exists but the value didn't exist,
  // returns true and set `value` to `default_value`. Else, returns true and
  // sets `value`.
  bool Traverse(
      int *from, const char *key, int32 *value, int32 default_value) const;

  // Put `key` and `value` pair into trie.
  void Put(const char *key, int32 value);

  // Saves the data into file. On success, returns true. Otherwise, returns
  // false
  bool Save(const char *filename);

  // Returns the size of array (in bytes)
  int size() const;

  // Checks arrays and blocks with std::assert, ONLY FOR TEST AND DEBUG!
  void _Check();

  // Use the external array. The external array is READ ONLY
  void SetArray(void *array);

  // Get the pointer of array data
  void *array() const;

 private:
  class Impl;
  Impl *impl_;
};

}  // namespace milkcat

#endif  // REIMU_TRIE_H_