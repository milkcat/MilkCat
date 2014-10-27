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
// perceptron_test.cc --- Created at 2014-10-23
//

#include "ml/feature_set.h"
#include "ml/multiclass_perceptron.h"
#include "ml/multiclass_perceptron_model.h"

#include <stdio.h>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include "utils/log.h"

using milkcat::FeatureSet;
using milkcat::MulticlassPerceptron;
using milkcat::MulticlassPerceptronModel;

void multiclass_perceptron_test(const char *train, const char *test, int iter) {
  char line[4096];
  std::istringstream iss;
  std::string label, feature;
  std::set<std::string> yset;

  // Get all y labels
  FILE *fd = fopen(train, "r");
  ASSERT(fd, (std::string("Unable to open file: ") + train).c_str());
  while (fgets(line, sizeof(line), fd) != NULL) {
    iss.str(line);
    ASSERT(iss >> label, "Unable to read label");
    yset.insert(label);
  }
  fclose(fd);

  // Create perceptron
  std::vector<std::string> yname(yset.begin(), yset.end());
  MulticlassPerceptronModel model(yname);
  MulticlassPerceptron perceptron(&model);
  printf("Create perceptron ... OK (y size = %d)\n",
         static_cast<int>(yname.size()));

  // Start training
  FeatureSet feature_set;
  for (int i = 0; i < iter; ++i) {
    // Training
    fd = fopen(train, "r");
    int correct = 0, total = 0;
    ASSERT(fd, (std::string("Unable to open file: ") + train).c_str());
    while (fgets(line, sizeof(line), fd) != NULL) {
      iss.clear();
      iss.str(line);
      ASSERT(iss >> label, "Unable to read label");
      feature_set.Clear();
      while (iss >> feature) {
        feature_set.Add(feature.c_str());
      }
      if (true == perceptron.Train(&feature_set, label.c_str())) {
        ++correct;
      }
      ++total;
    }
    fclose(fd);
    double p = static_cast<double>(correct) / total;
    printf("Iter %d: p = %.3lf correct = %d\n", i + 1, p, correct);
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s train-file test-file\n", argv[0]);
    return 1;
  }

  multiclass_perceptron_test(argv[1], argv[2], 20);
  return 0;
}