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
// final_rank.cc --- Created at 2014-02-07
//

#include <string>
#include <vector>
#include <algorithm>
#include "utils/utils.h"

namespace milkcat {

namespace {

// Reversed sort by second value
bool CmpPairByValue(const std::pair<std::string, double> &v1,
                    const std::pair<std::string, double> &v2) {
  return v1.second > v2.second;
}


void SortMapByValue(
    const utils::unordered_map<std::string, double> &map,
    std::vector<std::pair<std::string, double> > *sort_result) {
  sort_result->clear();
  for (utils::unordered_map<std::string, double>::const_iterator
       it = map.begin(); it != map.end(); ++it) {
    sort_result->push_back(*it);
  }
  sort(sort_result->begin(), sort_result->end(), CmpPairByValue);
}

}  // namespace

void FinalRank(
    const utils::unordered_map<std::string, double> &adjecent_entropy,
    const utils::unordered_map<std::string, double> &mutual_information,
    std::vector<std::pair<std::string, double> > *final_result,
    double remove_ratio = 0.1,
    double alpha = 0.6) {
  final_result->clear();

  // Remove the minimal N values in each map, the ratio is specified by
  // remove_ratio. And normalize the values in each map to [0, 1]
  std::vector<std::pair<std::string, double> > adjent_vec, mutinf_vec;
  SortMapByValue(adjecent_entropy, &adjent_vec);
  SortMapByValue(mutual_information, &mutinf_vec);

  int n_remove = adjent_vec.size() * remove_ratio;
  adjent_vec.erase(adjent_vec.end() - n_remove, adjent_vec.end());
  double max_value = adjent_vec[0].second;
  LOG("Adjent Max: %lf", max_value);
  for (int i = 0; i < adjent_vec.size(); ++i) 
    adjent_vec[i].second /= max_value;

  n_remove = mutinf_vec.size() * remove_ratio;
  mutinf_vec.erase(mutinf_vec.end() - n_remove, mutinf_vec.end());
  max_value = mutinf_vec[0].second;
  LOG("MI Max: %lf", max_value);
  for (int i = 0; i < mutinf_vec.size(); ++i) 
    mutinf_vec[i].second /= max_value;

  // Calculate the final value
  utils::unordered_map<std::string, double> adjent_new(
      adjent_vec.begin(), adjent_vec.end());
  utils::unordered_map<std::string, double> mutinf_new(
      mutinf_vec.begin(), mutinf_vec.end());
  utils::unordered_map<std::string, double> final;

  for (utils::unordered_map<std::string, double>::iterator
       it = adjent_new.begin(); it != adjent_new.end(); ++it) {
    if (mutinf_new.find(it->first) != mutinf_new.end()) {
      final[it->first] = alpha * it->second +
                         (1 - alpha) * mutinf_new[it->first];
      LOG("Word:%s Weight:%lf Adjent:%lf, MI: %lf",
          it->first.c_str(),
          final[it->first],
          it->second,
          mutinf_new[it->first]);
    }
  }

  SortMapByValue(final, final_result);
}

}  // namespace milkcat