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
// averaged_multiclass_perceptron.h --- Created at 2014-10-23
//

#ifndef SRC_ML_AVERAGED_MULTICLASS_PERCEPTRON_H_
#define SRC_ML_AVERAGED_MULTICLASS_PERCEPTRON_H_

#include "ml/multiclass_perceptron.h"
#include "ml/multiclass_perceptron_model.h"
#include <vector>

namespace milkcat {

template<class T> class PackedScore;

class AveragedMulticlassPerceptron: public MulticlassPerceptron {
 public:
  AveragedMulticlassPerceptron(MulticlassPerceptronModel *model);
  ~AveragedMulticlassPerceptron();

  // Increase count
  void IncCount();

  // Updates the cost of xid, yid, set cost[xid][yid] += `value`
  void UpdateCachedCost(int xid, int yid, float value);

  // Averaged the cost
  void FinishTrain();

 private:
  std::vector<PackedScore<float> *> cached_score_;
  int count_;
};

}  // namespace milkcat

#endif 
