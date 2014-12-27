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
// reimu_trie_test.cc --- Created at 2014-10-16
// Reimu x Marisa :P
//

#include "common/reimu_trie.h"

#include <assert.h>
#include <string>
#include <vector>

#ifdef BENCHMARK
#include <unordered_map>
#include <time.h>
#include "common/cedar.h"
#endif

#define N 10000
#define HALF_N 5000

using milkcat::ReimuTrie;

std::vector<std::string> putset;
std::vector<std::string> unputset;

void gen_random(char *s) {
  int len = rand() % 64;
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  s[len] = 0;
}

void generate_test_data() {
  char buff[128], key[128];
  for (int i = 0; i < N; ++i) {
    gen_random(buff);
    sprintf(key, "%s$%d",buff, i * 2);
    putset.push_back(key);
  }

  for (int i = 0; i < N; ++i) {
    gen_random(buff);
    sprintf(key, "%s$%d",buff, i * 2 + 1);
    unputset.push_back(key);
  }
}

void simple_get_put_test() {
  ReimuTrie trie;
  
  trie.Put("LARC-AMOD", 0);
  trie.Put("LARC-DEP", 1);

  trie.Put("LARC-NMOD", 2);
  trie.Put("LARC-P", 3);
  trie.Put("LARC-PMOD", 4);
  trie.Put("LARC-PRD", 5);
  trie.Put("LARC-SBAR", 6);
  trie.Put("LARC-SUB", 7);
  trie.Put("LARC-VC", 8);
  trie.Put("LARC-VMOD", 9);
  
  trie.Put("RARC-AMOD", 10);
  trie.Put("RARC-DEP", 11);
  trie.Put("RARC-NMOD", 12);
  
  trie.Put("RARC-OBJ", 13);
  trie.Put("RARC-P", 14);
  trie.Put("RARC-PMOD", 15);
  trie.Put("RARC-PRD", 16);
  trie.Put("RARC-ROOT", 17);
  trie.Put("RARC-SUB", 18);
  trie.Put("RARC-VC", 19);
  trie.Put("RARC-VMOD", 20);
  trie.Put("REDU", 21);
  trie.Put("SHIF", 22);
  trie._Check();

  int val;
  assert(trie.Get("LARC-DEP", -1) == 1);
  assert(trie.Get("RARC-NMOD", -1) == 12);

  puts("simple_get_put_test OK");
}

#ifdef BENCHMARK

void get_put_benchmark() {
  ReimuTrie *trie = new ReimuTrie();
  std::unordered_map<std::string, int> map;

  int start = clock();
  for (int i = 0; i < putset.size(); ++i) {
    trie->Put(putset[i].c_str(), i);
  }
  printf("ReimuTrie array_size = %d\n", trie->size() / 8);
  int end = clock();
  printf("ReimuTrie write: %.3f seconds.\n",
         static_cast<double>(end - start) / CLOCKS_PER_SEC);
  trie->_Check();

  int val;
  start = clock();
  for (int i = 0; i < putset.size(); ++i) {
    assert(trie->Get(putset[i].c_str(), &val));
    assert(val == i);
    assert(trie->Get(unputset[i].c_str(), &val) == false);
  }
  end = clock();
  printf("ReimuTrie read: %.3f seconds.\n", 
         static_cast<double>(end - start) / CLOCKS_PER_SEC);
  delete trie;

  start = clock();
  for (int i = 0; i < putset.size(); ++i) {
    map[putset[i]] = i;
  }
  end = clock();
  printf("unordered_map write: %.3f seconds.\n", 
         static_cast<double>(end - start) / CLOCKS_PER_SEC);

  start = clock();
  for (int i = 0; i < putset.size(); ++i) {
    assert(map[putset[i]] == i);
    assert(map.find(unputset[i]) == map.end());
  }
  end = clock();
  printf("unordered_map read: %.3f seconds.\n",
         static_cast<double>(end - start) / CLOCKS_PER_SEC);
  map = std::unordered_map<std::string, int>();

  cedar::da<int> *da = new cedar::da<int>();
  start = clock();
  for (int i = 0; i < putset.size(); ++i) {
    da->update(putset[i].c_str(), putset[i].size(), i);
  }
  end = clock();
  printf("cedar array_size = %lu\n", da->size());
  printf("cedar write: %.3f seconds.\n",
         static_cast<double>(end - start) / CLOCKS_PER_SEC);

  start = clock();
  for (int i = 0; i < putset.size(); ++i) {
    assert(da->exactMatchSearch<int>(putset[i].c_str()) == i);
    assert(da->exactMatchSearch<int>(unputset[i].c_str()) < 0);
  }
  end = clock();
  printf("cedar read: %.3f seconds.\n",
         static_cast<double>(end - start) / CLOCKS_PER_SEC);
  delete da;
}

#endif  // BENCHMARK

void save_and_open_test() {
  ReimuTrie *trie = new ReimuTrie();
  for (int i = 0; i < putset.size(); ++i) {
    trie->Put(putset[i].c_str(), i);
  }
  printf("array_size: %d\n", trie->size() / 8);
  assert(trie->Save("save.and.open.test.reimu_trie"));
  delete trie;

  trie = ReimuTrie::Open("save.and.open.test.reimu_trie");
  assert(trie);
  int val;
  for (int i = 0; i < putset.size(); ++i) {
    assert(trie->Get(putset[i].c_str(), -1) == i);
    assert(trie->Get(unputset[i].c_str(), -1) == -1);
  }
  delete trie;

  puts("save_and_open_test OK");
}

void restore_test() {
  ReimuTrie *trie = new ReimuTrie();
  for (int i = 0; i < HALF_N; ++i) {
    trie->Put(putset[i].c_str(), i);
  }
  assert(trie->Save("save.and.open.test.reimu_trie"));
  delete trie;

  trie = ReimuTrie::Open("save.and.open.test.reimu_trie");
  assert(trie);
  for (int i = HALF_N; i < N; ++i) {
    trie->Put(putset[i].c_str(), i);
  }
  trie->_Check();
  assert(trie->Save("save.and.open.test.reimu_trie"));
  delete trie;

  trie = ReimuTrie::Open("save.and.open.test.reimu_trie");
  assert(trie);
  int val;
  for (int i = 0; i < putset.size(); ++i) {
    assert(trie->Get(putset[i].c_str(), -1) == i);
    assert(trie->Get(unputset[i].c_str(), -1) == -1);
  }
  delete trie;

  puts("restore_test OK");
}

void set_array_test() {
  ReimuTrie *trie = new ReimuTrie();
  for (int i = 0; i < HALF_N; ++i) {
    trie->Put(putset[i].c_str(), i);
  }

  ReimuTrie *trie_ext = new ReimuTrie();
  trie_ext->SetArray(trie->array());  

  int val;
  for (int i = 0; i < HALF_N; ++i) {
    assert(trie_ext->Get(putset[i].c_str(), -1) == i);
    assert(val == i);
    assert(trie_ext->Get(unputset[i].c_str(), -1) == -1);
  }

  delete trie_ext;
  delete trie;
  puts("set_array_test OK");
}

int main() {
  generate_test_data();
  simple_get_put_test();
  save_and_open_test();
  restore_test();
  // set_array_test();

#ifdef BENCHMARK
  get_put_benchmark();
#endif

  return 0;
}