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

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

namespace milkcat {

namespace {

// Reversed sort by second value
bool CmpPairByValue(const std::pair<std::string, double> &v1,
                    const std::pair<std::string, double> &v2) {
  return v1.second > v2.second;
}

std::vector<std::pair<std::string, double>>
SortMapByValue(const std::unordered_map<std::string, double> &map) {
  std::vector<std::pair<std::string, double>> pair_vec(map.begin(), map.end());
  sort(pair_vec.begin(), pair_vec.end(), CmpPairByValue);

  return pair_vec;
}

}  // namespace

std::vector<std::pair<std::string, double>> FinalRank(
    const std::unordered_map<std::string, double> &adjecent_entropy,
    const std::unordered_map<std::string, double> &mutual_information,
    double remove_ratio = 0.1,
    double alpha = 0.6) {

  // Remove the minimal N values in each map, the ratio is specified by
  // remove_ratio. And normalize the values in each map to [0, 1]
  auto adjent_vec = SortMapByValue(adjecent_entropy);
  auto mutinf_vec = SortMapByValue(mutual_information);

  int n_remove = adjent_vec.size() * remove_ratio;
  adjent_vec.erase(adjent_vec.end() - n_remove, adjent_vec.end());
  int max_value = adjent_vec[0].second;
  for (auto &x : adjent_vec) x.second /= max_value;

  n_remove = mutinf_vec.size() * remove_ratio;
  mutinf_vec.erase(mutinf_vec.end() - n_remove, mutinf_vec.end());
  max_value = mutinf_vec[0].second;
  for (auto &x : mutinf_vec) x.second /= max_value;

  // Calculate the final value
  std::unordered_map<std::string, double> adjent_new(adjent_vec.begin(),
                                                     adjent_vec.end());
  std::unordered_map<std::string, double> mutinf_new(mutinf_vec.begin(),
                                                     mutinf_vec.end());
  std::unordered_map<std::string, double> final;

  for (auto &x : adjent_new) {
    if (mutinf_new.find(x.first) != mutinf_new.end()) {
      final[x.first] = alpha * x.second + (1 - alpha) * mutinf_new[x.first];
    }
  }

  return SortMapByValue(final);
}

}  // namespace milkcat
