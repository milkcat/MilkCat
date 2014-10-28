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
// multiclass_perceptron.cc --- Created at 2014-10-20
//

#define DEBUG

#include "ml/multiclass_perceptron.h"

#include <math.h>
#include <algorithm>
#include "ml/feature_set.h"
#include "ml/multiclass_perceptron_model.h"
#include "utils/status.h"
#include "utils/utils.h"

namespace milkcat {

const char *MulticlassPerceptron::yname(int yid) const { 
  return model_->yname(yid);
}

int MulticlassPerceptron::ysize() const {
  return model_->ysize();
}

MulticlassPerceptron::MulticlassPerceptron(MulticlassPerceptronModel *model): 
    model_(model) {
  ycost_ = new float[model->ysize()];
}

MulticlassPerceptron::~MulticlassPerceptron() {
  delete ycost_;
  ycost_ = NULL;
}

int MulticlassPerceptron::Classify(const FeatureSet *feature_set) {
  // Clear the y_cost_ array
  for (int i = 0; i < ysize(); ++i) ycost_[i] = 0.0;
  for (int i = 0; i < feature_set->size(); ++i) {
    int xid = model_->xid(feature_set->at(i));
    if (xid >= 0) {
      for (int yid = 0; yid < ysize(); ++yid) {
        ycost_[yid] += model_->cost(xid, yid);
      }
    }
  }

  // To find the maximum cost of y
  float *maximum_y = std::max_element(ycost_, ycost_ + ysize());
  int maximum_yid = maximum_y - ycost_;
  return maximum_yid;  
}

bool MulticlassPerceptron::Train(const FeatureSet *feature_set,
                                 const char *label) {
  int predict_yid = Classify(feature_set);
  int correct_yid = model_->yid(label);
  ASSERT(correct_yid >= 0, "Unexcpected label");

  // Increase in count (for Averaged multiclass perceptron)
  IncCount();

  if (predict_yid != correct_yid) {
    // Update: w = w + F(x, y) - F(x, y_)
    for (int i = 0; i < feature_set->size(); ++i) {
      // LOG(feature_set->at(i));
      int xid = model_->GetOrInsertXId(feature_set->at(i));
      
      model_->set_cost(xid, correct_yid, model_->cost(xid, correct_yid) + 1.0);
      model_->set_cost(xid, predict_yid, model_->cost(xid, predict_yid) - 1.0);

      UpdateCachedCost(xid, correct_yid, 1.0f);
      UpdateCachedCost(xid, predict_yid, -1.0f);
    }
    return false;
  } else {
    return true;
  }
}

// Do nothing in normal multiclass perceptron
void MulticlassPerceptron::UpdateCachedCost(int, int, float) {}
void MulticlassPerceptron::IncCount() {}
void MulticlassPerceptron::FinishTrain() {}

}  // namespace milkcat