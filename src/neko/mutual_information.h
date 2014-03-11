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
// mutual_information.h --- Created at 2014-02-06
//

#ifndef SRC_NEKO_MUTUAL_INFORMATION_H_
#define SRC_NEKO_MUTUAL_INFORMATION_H_

#include <unordered_map>
#include <string>
#include "utils/status.h"

namespace milkcat {

std::unordered_map<std::string, double> GetMutualInformation(
    const std::unordered_map<std::string, int> &bigram_vocab,
    const std::unordered_map<std::string, float> &candidate,
    Status *status);

}  // namespace milkcat

#endif  // SRC_NEKO_MUTUAL_INFORMATION_H_
