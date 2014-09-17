//
// feature_extractor.h --- Created at 2013-10-09
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

#ifndef SRC_COMMON_FEATURE_EXTRACTOR_H_
#define SRC_COMMON_FEATURE_EXTRACTOR_H_

#include <stdlib.h>
#include "common/milkcat_config.h"

namespace milkcat {

class FeatureExtractor {
 public:
  virtual void ExtractFeatureAt(size_t position,
                                char (*feature_list)[kFeatureLengthMax],
                                int list_size) = 0;
  virtual size_t size() const = 0;
  virtual ~FeatureExtractor();
};

inline FeatureExtractor::~FeatureExtractor() {}

}  // namespace milkcat

#endif  // SRC_COMMON_FEATURE_EXTRACTOR_H_
