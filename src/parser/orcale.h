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
// orcale.h --- Created at 2014-10-10
//

#ifndef SRC_PARSER_ORCALE_H_
#define SRC_PARSER_ORCALE_H_

#include <vector>
#include "common/milkcat_config.h"

namespace milkcat {

class DependencyInstance;

namespace utils {
class StringBuilder;
}

// Given the dependency, output each transitions.
class Orcale {
 public:
  Orcale();
  ~Orcale();

  // Parses the `instance`
  void Parse(const DependencyInstance *instance);

  // Returns the label of next transition. When no more transitions, returns
  // NULL
  const char *Next();

 private:
  std::vector<int> stack_;
  const DependencyInstance *instance_;
  char transition_label_[kLabelSizeMax];
  utils::StringBuilder *string_builer_;
  int input_ptr_;

  // Returns the head node id and dependency label of `nodeid`
  int Head(int nodeid);
  const char *Label(int nodeid);
  
  // If current state needs a reduce transition
  bool IsReduce();
};

}  // namespace milkcat

#endif