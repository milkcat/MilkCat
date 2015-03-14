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

#include "ml/perceptron.h"

#include <math.h>
#include <algorithm>
#include "ml/feature_set.h"
#include "ml/perceptron_model.h"
#include "ml/packed_score.h"
#include "util/status.h"
#include "util/util.h"

namespace milkcat {

const char *Perceptron::yname(int yid) const { 
  return model_->yname(yid);
}

int Perceptron::ysize() const {
  return model_->ysize();
}

Perceptron::Perceptron(PerceptronModel *model): 
    model_(model),
    sample_count_(0) {
  ycost_ = new float[model->ysize()];
}

Perceptron::~Perceptron() {
  delete[] ycost_;
  ycost_ = NULL;

  for (std::vector<PackedScore<float> *>::iterator
       it = cached_score_.begin(); it != cached_score_.end(); ++it) {
    delete *it;
    *it = NULL;
  }
}

int Perceptron::Classify(const FeatureSet *feature_set) {
  PackedScore<float>::Iterator it;

  // Clear the y_cost_ array
  for (int i = 0; i < ysize(); ++i) ycost_[i] = 0.0;

  for (int i = 0; i < feature_set->size(); ++i) {
    int xid = model_->xid(feature_set->at(i));
    if (xid >= 0) {
      PackedScore<float> *score = model_->get_score(xid);
      score->Begin(&it);
      while (score->HasNext(it)) {
        std::pair<int, float> one_score = score->Next(&it);
        ycost_[one_score.first] += one_score.second;
      }
    }
  }

  // To find the maximum cost of y
  float *maximum_y = std::max_element(ycost_, ycost_ + ysize());
  int maximum_yid = static_cast<int>(maximum_y - ycost_);
  return maximum_yid;  
}
void Perceptron::Update(const FeatureSet *feature_set, int yid, float value) {
  for (int i = 0; i < feature_set->size(); ++i) {
    int xid = model_->GetOrInsertXId(feature_set->at(i));
    PackedScore<float> *score = model_->get_score(xid);
    score->Add(yid, value);
    UpdateCachedScore(xid, yid, value);
  }  
}

bool Perceptron::Train(const FeatureSet *feature_set,
                                 const char *label) {
  int predict_yid = Classify(feature_set);
  int correct_yid = model_->yid(label);
  MC_ASSERT(correct_yid >= 0, "unexcpected label");
  // printf("Train: %s\n", label);
  // printf("Predict: %s\n", model_->yname(predict_yid));

  if (predict_yid != correct_yid) {
    Update(feature_set, correct_yid, 1.0f);
    Update(feature_set, predict_yid, -1.0f);

    return false;
  } else {
    return true;
  }
}

void Perceptron::UpdateCachedScore(int xid, int yid, float value) {
  while (xid >= static_cast<int>(cached_score_.size())) {
    cached_score_.push_back(new PackedScore<float>());
  }

  PackedScore<float> *score = cached_score_[xid];
  score->Add(yid, sample_count_ * value);
}

void Perceptron::IncreaseSampleCount() {
  sample_count_++;
}

void Perceptron::FinishTrain() {
  PackedScore<float>::Iterator it;
  for (int xid = 0; xid < model_->xsize(); ++xid) {
    PackedScore<float> *cached_score = cached_score_[xid],
                       *score = model_->get_score(xid);
    cached_score->Begin(&it);
    while (cached_score->HasNext(it)) {
      std::pair<int, float> one_score = cached_score->Next(&it);
      float add = one_score.second / sample_count_;
      score->Add(one_score.first, -add);
    }
  }
}

}  // namespace milkcat
