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
// rank.cc --- Created at 2014-06-24
//

#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include "utils/log.h"
#include "utils/utils.h"

namespace milkcat {

const double kRankThreshold = -1.0;

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

void NewwordRank(
    const utils::unordered_map<std::string, double> &adjecent_entropy,
    const utils::unordered_map<std::string, double> &mutual_information,
    std::vector<std::pair<std::string, double> > *newword_rank,
    double alpha = 0.6) {
  // Calculate the mean of adjacent entropy
  double adjent_mean = 0.0;
  for (utils::unordered_map<std::string, double>::const_iterator
       it = adjecent_entropy.begin(); it != adjecent_entropy.end(); ++it) {
    adjent_mean += it->second;
  }
  adjent_mean /= adjecent_entropy.size();

  // Calculate the variance of adjacent entrppy
  double adjent_variance = 0.0;
  for (utils::unordered_map<std::string, double>::const_iterator
       it = adjecent_entropy.begin(); it != adjecent_entropy.end(); ++it) {
    adjent_variance += pow(it->second - adjent_mean, 2);
  }
  adjent_variance /= adjecent_entropy.size();
  double adjent_sigma = sqrt(adjent_variance);

  // Calculate the mean of PMI (pointwise mutual entropy)
  double PMI_mean = 0.0;
  for (utils::unordered_map<std::string, double>::const_iterator
       it = mutual_information.begin(); it != mutual_information.end(); ++it) {
    PMI_mean += it->second;
  }
  PMI_mean /= mutual_information.size();

  // Calculate the variance of PMI
  double PMI_variance = 0.0;
  for (utils::unordered_map<std::string, double>::const_iterator
       it = mutual_information.begin(); it != mutual_information.end(); ++it) {
    PMI_variance += pow(it->second - PMI_mean, 2);
  }
  PMI_variance /= mutual_information.size();
  double PMI_sigma = sqrt(PMI_variance);  

  // Calculate the weight for all newwords
  newword_rank->clear();
  double PMI_normalized, adjent_normalized, weight;
  utils::unordered_map<std::string, double> weight_map;
  for (utils::unordered_map<std::string, double>::const_iterator
       it = adjecent_entropy.begin(); it != adjecent_entropy.end(); ++it) {
    utils::unordered_map<std::string, double>::const_iterator 
    PMI_it = mutual_information.find(it->first);
    if (PMI_it != mutual_information.end()) {
      adjent_normalized = (it->second - adjent_mean) / adjent_sigma;
      PMI_normalized = (PMI_it->second - PMI_mean) / PMI_sigma;

      // Ignore the words that the normalized PMI or adjent less than
      // kRankThreshold
      if (adjent_normalized > kRankThreshold &&
          PMI_normalized > kRankThreshold) {
        weight = alpha * adjent_normalized + (1- alpha) * PMI_normalized;
        weight_map[it->first] = weight;
        LOG("Word:", it->first, ", ",
            "Weight:", weight, ", ",
            "AdjEnt:", adjent_normalized << ", ",
            "PMI: ", PMI_normalized);
      }
    }
  }

  SortMapByValue(weight_map, newword_rank);
}

}  // namespace milkcat