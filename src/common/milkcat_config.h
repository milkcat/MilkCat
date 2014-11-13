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
// config.h
// milkcat_config.h --- Created at 2013-09-17
//

#ifndef SRC_COMMON_MILKCAT_CONFIG_H_
#define SRC_COMMON_MILKCAT_CONFIG_H_

#include <stdlib.h>

namespace milkcat {

enum {
  kTokenMax = 4096,
  kFeatureLengthMax = 100,
  kTermLengthMax = kFeatureLengthMax,
  kPOSTagLengthMax = 10,
  kHMMSegmentAndPOSTaggingNBest = 3,
  kUserTermIdStart = 0x40000000,
  kHmmModelMagicNumber = 0x3322,
  kMulticlassPerceptronModelMagicNumber = 0x1a1a,
  kLabelSizeMax = 64
};

const float kDefaultCost = 6.0;

}  // namespace milkcat

#endif  // SRC_COMMON_MILKCAT_CONFIG_H_
