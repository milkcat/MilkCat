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
// Part of this code comes from CRF++
//
// CRF++ -- Yet Another CRF toolkit
// Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
// crfpp_model.h --- Created at 2013-10-28
// crf_model.h --- Created at 2013-11-02
//

#ifndef SRC_PARSER_CRF_MODEL_H_
#define SRC_PARSER_CRF_MODEL_H_

#include <string>
#include <vector>
#include "common/static_array.h"
#include "util/util.h"

namespace milkcat {

class ReimuTrie;
template<class T> class StaticArray;

class CRFModel {
 public:
  // Open a CRF++ model file
  static CRFModel *New(const char *model_path, Status *status);
  static CRFModel *OpenText(const char *text_filename,
                            const char *template_filename,
                            Status *status);
  void Save(const char *model_prefix, Status *status);

  ~CRFModel();

  // Get id of `xname`, returns -1 when `xname` didn't exist in index
  int xid(const char *xname) const;

  // Get Tag's string text by its id
  const char *yname(int yid) const {
    return y_[yid].c_str();
  }

  // Gets id by yname, return -1 if it didn't exist
  int yid(const char *yname) const  {
    for (std::vector<std::string>::const_iterator
         it = y_.begin(); it != y_.end(); ++it) {
      if (*it == yname) return it - y_.begin();
    }

    return -1;
  }

  // Templates
  const char *unigram_template(int index) const {
    return unigram_tmpl_[index].c_str();
  }
  const char *bigram_template(int index) const {
    return bigram_tmpl_[index].c_str();
  }
  int unigram_template_num() const { return unigram_tmpl_.size(); }
  int bigram_template_num() const { return bigram_tmpl_.size(); }

  // Get the number of tag
  int ysize() const { return y_.size(); }

  // Get the cost for feature with current tag
  double unigram_cost(int xid, int yid) const {
    return unigram_cost_->get(xid * ysize() + yid);
  }

  // Get the bigram cost for feature with left tag and right tag
  double bigram_cost(int xid, int left_yid, int right_yid) const {
    int idx = xid * ysize() * ysize() + left_yid * ysize() + right_yid;
    return bigram_cost_->get(idx);
  }

 private:
  std::vector<std::string> y_;
  std::vector<std::string> unigram_tmpl_;
  std::vector<std::string> bigram_tmpl_;
  ReimuTrie *xindex_;
  StaticArray<float> *unigram_cost_;
  StaticArray<float> *bigram_cost_;
  int bigram_xsize_;
  int unigram_xsize_;
  
  CRFModel();
};

}  // namespace milkcat

#endif  // SRC_PARSER_CRF_MODEL_H_
