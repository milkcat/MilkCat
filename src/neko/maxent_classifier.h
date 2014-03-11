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
// maxent_classifier.h --- Created at 2014-01-31
//

#ifndef SRC_NEKO_MAXENT_CLASSIFIER_H_
#define SRC_NEKO_MAXENT_CLASSIFIER_H_

#include <unordered_map>
#include <string>
#include <vector>
#include "utils/status.h"
#include "milkcat/darts.h"

namespace milkcat {

class MaxentModel {
 public:
  // Load and create the maximum entropy model data from the model file
  // specified by model_path. New loads binary model file and NewFromText loads
  // model file of text format. On success, return the instance and status =
  // Status::OK(). On failed, return nullptr and status != Status::OK()
  static MaxentModel *NewFromText(const char *model_path, Status *status);
  static MaxentModel *New(const char *model_path, Status *status);

  MaxentModel();
  ~MaxentModel();

  // Save the model data into file
  void Save(const char *model_path, Status *status);

  // Get the cost of a feature with its y-tag in maximum entropy model
  float cost(int yid, int feature_id) {
    return cost_[feature_id * ysize_ + yid];
  }

  // If the feature_str not exists in feature set, use this value instead
  static constexpr int kFeatureIdNone = -1;
  static constexpr int kYNameMax = 64;

  // Get the number of y-tags in the model
  int ysize() const { return ysize_; }

  // Get the name string of a y-tag's id
  const char *yname(int yid) const { return yname_[yid]; }

  // Return the id of feature_str, if the feature_str not in feature set of
  // model return kFeatureIdNone
  int feature_id(const char *feature_str) const {
    int id = double_array_.exactMatchSearch<int>(feature_str);
    if (id >= 0)
      return id;
    else
      return kFeatureIdNone;
  }

 private:
  Darts::DoubleArray double_array_;
  char *index_data_;
  int xsize_;
  int ysize_;
  char (*yname_)[kYNameMax];
  float *cost_;
};

class MaxentClassifier {
 public:
  explicit MaxentClassifier(MaxentModel *model);
  ~MaxentClassifier();

  // Classify an instance of features specified by feature_list and return the
  // y-tag for the instance
  const char *Classify(const std::vector<std::string> &feature_list) const;

 private:
  MaxentModel *model_;
  double *y_cost_;
  int y_size_;
};

}  // namespace milkcat

#endif  // SRC_NEKO_MAXENT_CLASSIFIER_H_
