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
void MulticlassPerceptron::Update(const FeatureSet *feature_set,
                                  int yid,
                                  float value) {
  for (int i = 0; i < feature_set->size(); ++i) {
    int xid = model_->GetOrInsertXId(feature_set->at(i));
    
    model_->set_cost(xid, yid, model_->cost(xid, yid) + value);
    UpdateCachedCost(xid, yid, value);
  }  
}

void MulticlassPerceptron::Train(const FeatureSet *feature_set,
                                 int correct_yid,
                                 int incorrect_yid) {
  // Update: w = w + F(x, y) - F(x, y_)
  Update(feature_set, correct_yid, 1.0f);
  Update(feature_set, incorrect_yid, -1.0f);
}

bool MulticlassPerceptron::Train(const FeatureSet *feature_set,
                                 const char *label) {
  int predict_yid = Classify(feature_set);
  int correct_yid = model_->yid(label);
  ASSERT(correct_yid >= 0, "Unexcpected label");
  // printf("Train: %s\n", label);
  // printf("Predict: %s\n", model_->yname(predict_yid));

  if (predict_yid != correct_yid) {
    Train(feature_set, correct_yid, predict_yid);
    // puts("Train: UPDATE");
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