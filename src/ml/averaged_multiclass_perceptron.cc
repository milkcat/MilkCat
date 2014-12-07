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

#include "ml/averaged_multiclass_perceptron.h"
#include "utils/utils.h"

namespace milkcat {

AveragedMulticlassPerceptron::AveragedMulticlassPerceptron(
    MulticlassPerceptronModel *model):
        MulticlassPerceptron(model),
        count_(0) {
}

AveragedMulticlassPerceptron::~AveragedMulticlassPerceptron() {
}

// Increase count
void AveragedMulticlassPerceptron::IncCount() {
  count_ += 1;
}

void AveragedMulticlassPerceptron::UpdateCachedCost(
    const char *xname, int yid, float value) {
  if (cached_cost_.find(xname) == cached_cost_.end()) {
    cached_cost_[xname].resize(model_->ysize());
  }
  cached_cost_[xname][yid] += count_ * value;
}

void AveragedMulticlassPerceptron::FinishTrain() {
  int i = 0;
  for (unordered_map<std::string, std::vector<float> >::iterator
       it = cached_cost_.begin(); it != cached_cost_.end(); ++it) {
    printf("i = %d\n", ++i);
    for (int yid = 0; yid < model_->ysize(); ++yid) {
      if (it->second[yid] == 0.0) continue;
      int xid = model_->xid(it->first.c_str());
      assert(xid > 0);
      float cost = model_->cost(xid, yid);
      model_->set_cost(it->first.c_str(),
                       yid,
                       cost - it->second[yid] / count_);
    }
  }
}

}  // namespace milkcat