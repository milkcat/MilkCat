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
// multiclass_perceptron_model.h --- Created at 2014-10-19
//

#ifndef SRC_ML_MULTICLASS_PERCEPTRON_MODEL_H_
#define SRC_ML_MULTICLASS_PERCEPTRON_MODEL_H_

#include <assert.h>
#include <string>
#include <vector>
#include <stdio.h>

namespace milkcat {

class Status;
class ReimuTrie;
template<class T> class PackedScore;

// The model class used in MulticlassPerceptron
class MulticlassPerceptronModel {
 public:
  // Loads the multiclass perceptron model data from `filename`.
  static MulticlassPerceptronModel *OpenText(const char *filename,
                                             Status *status);
  static MulticlassPerceptronModel *Open(const char *filename, Status *status);

  // Creates a new `MulticlassPerceptronModel` with specified labels (y)
  MulticlassPerceptronModel(const std::vector<std::string> &y);
  ~MulticlassPerceptronModel();

  // Save the model data into binary file `filename`
  void Save(const char *filename, Status *status);

  // If the feature_str does not exists in feature set, use this value instead
  enum {
    kIdNone = -1,
  };

  // Get the number of labels(y) in the model
  int ysize() const { return yname_.size(); }
  int xsize() const { return score_.size(); }

  // Gets label name by id or gets label id by name
  // If the name is not in the model, `GetOrInsertYId` inserts the label into
  // model and returns the inserted label id and `yid` just returns `kIdNone`
  const char *yname(int yid) const { return yname_[yid].c_str(); }
  int yid(const char *yname) const;

  // Returns the id of feature string, if the feature string is not in the
  // model. `GetOrInsertXId` inserts the feature string in the model, `xid`
  // retutns `kIdNone`
  int GetOrInsertXId(const char *xname);
  int xid(const char *xname) const;

  // Gets the scores of `xid`
  PackedScore<float> *get_score(int xid);

 private:
  ReimuTrie *xindex_;
  int xsize_;

  ReimuTrie *yindex_;

  std::vector<PackedScore<float> *> score_;
  std::vector<std::string> yname_;
};

}  // namespace MilkCat

#endif  // SRC_ML_MULTICLASS_PERCEPTRON_MODEL_H_
