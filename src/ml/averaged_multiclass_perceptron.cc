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
#include "ml/packed_score.h"
#include "utils/utils.h"

namespace milkcat {

AveragedMulticlassPerceptron::AveragedMulticlassPerceptron(
    MulticlassPerceptronModel *model):
        MulticlassPerceptron(model),
        count_(0) {
}

AveragedMulticlassPerceptron::~AveragedMulticlassPerceptron() {
  for (std::vector<PackedScore<float> *>::iterator
       it = cached_score_.begin(); it != cached_score_.end(); ++it) {
    delete *it;
    *it = NULL;
  }
}

// Increase count
void AveragedMulticlassPerceptron::IncCount() {
  count_ += 1;
}

void AveragedMulticlassPerceptron::UpdateCachedCost(
    int xid, int yid, float value) {
  while (xid >= cached_score_.size()) {
    cached_score_.push_back(new PackedScore<float>(model_->ysize()));
  }

  PackedScore<float> *score = cached_score_[xid];
  score->Add(yid, count_ * value);
}

void AveragedMulticlassPerceptron::FinishTrain() {
  PackedScore<float>::Iterator it;
  for (int xid = 0; xid < model_->xsize(); ++xid) {
    PackedScore<float> *cached_score = cached_score_[xid],
                         *score = model_->get_score(xid);
    cached_score->Begin(&it);
    while (cached_score->HasNext(it)) {
      std::pair<int, float> one_score = cached_score->Next(&it);
      float add = one_score.second / count_;
      score->Add(one_score.first, -add);
    }
  }
}

}  // namespace milkcat
