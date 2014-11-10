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
// orcale_test.cc --- Created at 2014-10-21
//

#include <assert.h>
#include "parser/tree_instance.h"
#include "parser/orcale.h"

void orcale_test() {
  TreeInstance *instance = new TreeInstance();
  instance->set_value_at(0, "NMOD", 2);
  instance->set_value_at(1, "SBJ", 3);
  instance->set_value_at(2, "ROOT", 0);
  instance->set_value_at(3, "NMOD", 5);
  instance->set_value_at(4, "OBJ", 3);
  instance->set_value_at(5, "NMOD", 5);
  instance->set_value_at(6, "NMOD", 8);
  instance->set_value_at(7, "PMOD", 6);
  instance->set_value_at(8, "P", 3);
  instance->set_size(9);

  Orcale *orcale = new Orcale();
  const char *label;
  orcale->Parse(instance);
  assert((label = orcale->Next()) && strcmp(label, "shift") == 0);
  assert((label = orcale->Next()) && strcmp(label, "leftarc_NMOD") == 0);
  assert((label = orcale->Next()) && strcmp(label, "shift") == 0);
  assert((label = orcale->Next()) && strcmp(label, "leftarc_SBJ") == 0);
  assert((label = orcale->Next()) && strcmp(label, "rightarc_ROOT") == 0);
  assert((label = orcale->Next()) && strcmp(label, "shift") == 0);
  assert((label = orcale->Next()) && strcmp(label, "leftarc_NMOD") == 0);
  assert((label = orcale->Next()) && strcmp(label, "rightarc_OBJ") == 0);
  assert((label = orcale->Next()) && strcmp(label, "rightarc_NMOD") == 0);
  assert((label = orcale->Next()) && strcmp(label, "shift") == 0);
  assert((label = orcale->Next()) && strcmp(label, "leftarc_NMOD") == 0);
  assert((label = orcale->Next()) && strcmp(label, "rightarc_PMOD") == 0);
  assert((label = orcale->Next()) && strcmp(label, "reduce") == 0);
  assert((label = orcale->Next()) && strcmp(label, "reduce") == 0);
  assert((label = orcale->Next()) && strcmp(label, "reduce") == 0);
  assert((label = orcale->Next()) && strcmp(label, "rightarc_P") == 0);
  assert((label = orcale->Next()) && strcmp(label, "reduce") == 0);
  assert((label = orcale->Next()) && strcmp(label, "reduce") == 0);
  assert(orcale->Next() == NULL);

  delete instance;
  delete orcale;
}

int main() {
  orcale_test();
}