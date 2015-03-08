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
// hmm_segment_and_pos_tagger.h --- Created at 2013-08-14
// bigram_segmenter.h --- Created at 2013-10-21
//

#ifndef SRC_SEGMENTER_BIGRAM_SEGMENTER_H_
#define SRC_SEGMENTER_BIGRAM_SEGMENTER_H_

#include <stdint.h>
#include <set>
#include "common/milkcat_config.h"
#include "common/static_array.h"
#include "common/static_hashtable.h"
#include "ml/beam.h"
#include "segmenter/segmenter.h"
#include "util/pool.h"

namespace milkcat {

class ReimuTrie;
class TokenInstance;
class TermInstance;
class Status;
class Model;

class BigramSegmenter: public Segmenter {
 private:
  // Stores the state in traversing the trie
  struct TraverseState;

 public:
  // A node in decode graph
  struct Node;

  // Create the bigram segmenter from a model factory. On success, return an
  // instance of BigramSegmenter. On failed, return NULL and set status
  // a failed value
  static BigramSegmenter *New(Model *model_factory,
                              bool use_bigram,
                              Status *status);

  ~BigramSegmenter();

  // Segment a token instance into term instance
  void Segment(TermInstance *term_instance, TokenInstance *token_instance);

 private:
  class NodeComparator;

  static const int kDefaultBeamSize = 3;
  // Number of Node in each buckets_
  int beam_size_;

  // Buckets contain nodes for viterbi decoding
  Beam<Node, NodeComparator> *lattice_[kTokenMax + 1];

  // NodePool instance to alloc and release node
  Pool<Node> *node_pool_;

  // Costs for unigram and bigram.
  const StaticArray<float> *unigram_cost_;
  const StaticArray<float> *user_cost_;
  const StaticHashTable<int64_t, float> *bigram_cost_;

  // Index for words in dictionary
  const ReimuTrie *index_;
  const ReimuTrie *user_index_;
  bool has_user_index_;


  BigramSegmenter();

  double CalculateBigramCost(int left_id,
                             int right_id,
                             double left_cost,
                             double right_cost);

  int GetTermIdAndUnigramCost(
      const char *token_string,
      TraverseState *traverse_state,
      double *right_cost);

  // Adds possible term to the lattice at `position`
  void AddPossibleTermToLattice(TokenInstance *token_instance, int position);

  // Finds the best result from lattice and stores into term_instance 
  void StoreResult(TermInstance *term_instance,
                   TokenInstance *token_instance);
};

}  // namespace milkcat

#endif  // SRC_SEGMENTER_BIGRAM_SEGMENTER_H_
