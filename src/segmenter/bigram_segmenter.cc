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
// hmm_segment_and_pos_tagger.cc --- Created at 2013-08-15
// bigram_segmenter.cc --- Created at 2013-10-23
//

#include "segmenter/bigram_segmenter.h"

#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <string>
#include "libmilkcat.h"
#include "common/milkcat_config.h"
#include "common/model_impl.h"
#include "common/trie_tree.h"
#include "include/milkcat.h"
#include "segmenter/term_instance.h"
#include "tokenizer/token_instance.h"
#include "utils/log.h"
#include "utils/utils.h"

namespace milkcat {

struct BigramSegmenter::Node {
  int beam_id;            // The bucket contains this node
  int term_id;            // term_id for this node
  double cost;            // Cost in this path  
  const Node *from_node;  // Previous node pointer
  int term_position;      // Position in a term_instance

  inline void 
  set_value(int beam_id, int term_id, double cost, const Node *from_node) {
    this->beam_id = beam_id;
    this->term_id = term_id;
    this->cost = cost;
    this->from_node = from_node;

    if (from_node) {
      this->term_position = from_node->term_position + 1;
    } else {
      this->term_position = -1;
    }
  }
};

namespace {

// Compare two Node in cost
static inline bool NodePtrCmp(BigramSegmenter::Node *n1,
                              BigramSegmenter::Node *n2) {
  return n1->cost < n2->cost;
}

}  // namespace

BigramSegmenter::BigramSegmenter(): beam_size_(0),
                                    node_pool_(NULL),
                                    unigram_cost_(NULL),
                                    user_cost_(NULL),
                                    bigram_cost_(NULL),
                                    index_(NULL),
                                    user_index_(NULL),
                                    has_user_index_(false),
                                    use_disabled_term_ids_(false) {
}

BigramSegmenter::~BigramSegmenter() {
  delete node_pool_;
  node_pool_ = NULL;

  for (int i = 0; i < sizeof(beams_) / sizeof(Beam<Node> *); ++i) {
    delete beams_[i];
    beams_[i] = NULL;
  }
}

BigramSegmenter *BigramSegmenter::New(Model::Impl *model_factory,
                                      bool use_bigram,
                                      Status *status) {
  BigramSegmenter *self = new BigramSegmenter();

  self->beam_size_ = use_bigram? kDefaultBeamSize: 1;
  self->node_pool_ = new utils::Pool<Node>();

  // Initialize the beams_
  for (int i = 0; i < sizeof(self->beams_) / sizeof(Beam<Node> *); ++i) {
    self->beams_[i] = new Beam<Node>(self->beam_size_,
                                     self->node_pool_,
                                     i,
                                     NodePtrCmp);
  }

  self->index_ = model_factory->Index(status);
  if (status->ok() && model_factory->HasUserDictionary()) {
    self->has_user_index_ = true;
    self->user_index_ = model_factory->UserIndex(status);
    if (status->ok()) self->user_cost_ = model_factory->UserCost(status);
  }

  if (status->ok()) self->unigram_cost_ = model_factory->UnigramCost(status);
  if (status->ok() && use_bigram == true) 
    self->bigram_cost_ = model_factory->BigramCost(status);

  // Default is not use disabled term-ids
  self->use_disabled_term_ids_ = false;

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

// Traverse the system and user index to find the term_id at current position,
// then get the unigram cost for the term-id. Return the term-id and stores the
// cost in double &unigram_cost
// NOTE: If the word in current position exists both in system dictionary and
// user dictionary, returns the term-id in system dictionary and stores the cost
// of user dictionary into unigram_cost if its value is not kDefaultCost
inline int BigramSegmenter::GetTermIdAndUnigramCost(
    const char *token_str,
    bool *system_flag,
    bool *user_flag,
    size_t *system_node,
    size_t *user_node,
    double *right_cost) {

  int term_id = TrieTree::kNone,
      uterm_id = TrieTree::kNone;

  if (*system_flag) {
    term_id = index_->Traverse(token_str, system_node);
    if (term_id == TrieTree::kNone) *system_flag = false;
    if (term_id >= 0) {
      *right_cost = unigram_cost_->get(term_id);
      LOG("System unigram find " << term_id << " " << *right_cost);
    }
  }

  if (*user_flag) {
    uterm_id = user_index_->Traverse(token_str, user_node);
    if (uterm_id == TrieTree::kNone) *user_flag = false;

    if (uterm_id >= 0) {
      double cost = user_cost_->get(uterm_id - kUserTermIdStart);
      LOG("User unigram find " << uterm_id << " " << cost);

      if (term_id < 0) {
        *right_cost = cost;

        // Use term id in user dictionary iff term-id in system dictionary not
        // exist
        term_id = uterm_id;
      } else if (cost != kDefaultCost) {
        // If word exists in system dictionary only when the cost in user
        // dictionary not equals kDefaultCost, stores the cost to right_cost
        *right_cost = cost;
      }
    }
  }

  // If term-id in disabled list, set term_id to TrieTree::kNone
  if (use_disabled_term_ids_ == true) {
    if (disabled_term_ids_.find(term_id) != disabled_term_ids_.end()) {
      term_id = TrieTree::kNone;
    }
  }

  return term_id;
}

// Calculates the cost form left word-id to right term-id in bigram model. The
// cost equals -log(p(right_word|left_word)). If no bigram data exists, use
// unigram model cost = -log(p(right_word))
inline double BigramSegmenter::CalculateBigramCost(int left_id,
                                                   int right_id,
                                                   double left_cost,
                                                   double right_cost) {
  double cost;

  // If bigram is disabled
  if (bigram_cost_ == NULL) return left_cost + right_cost;

  int64_t key = (static_cast<int64_t>(left_id) << 32) + right_id;
  const float *it = bigram_cost_->Find(key);
  if (it != NULL) {
    // if have bigram data use p(x_n+1|x_n) = p(x_n+1, x_n) / p(x_n)
    cost = left_cost + (*it - unigram_cost_->get(left_id));
    LOG("bigram find " << left_id << " "<< right_id << " " << cost - left_cost);
  } else {
    cost = left_cost + right_cost;
  }

  return cost;
}

int BigramSegmenter::GetTermId(const char *term_str) {
  bool system_flag = true;
  bool user_flag = has_user_index_;
  size_t system_node = 0;
  size_t user_node = 0;
  double cost = 0.0;

  return GetTermIdAndUnigramCost(term_str,
                                 &system_flag,
                                 &user_flag,
                                 &system_node,
                                 &user_node,
                                 &cost);
}

void BigramSegmenter::BuildBeamFromPosition(TokenInstance *token_instance, 
                                            int position) {
  size_t index_node = 0,
         user_node = 0;
  bool index_flag = true, 
       user_flag = has_user_index_;
  double cost, right_cost;
  const Node *node = NULL;
  Node *new_node = NULL;

  beams_[position]->Shrink();
  const char *token_str = NULL;
  int length_end = token_instance->size() - position;
  for (int length = 0; length < length_end; ++length) {
    LOG("Position: [" << position << ", " << position + length + 1 << ")");

    // Get current term-id from system and user dictionary
    token_str = token_instance->token_text_at(position + length);
    int term_id = GetTermIdAndUnigramCost(token_str,
                                          &index_flag,
                                          &user_flag,
                                          &index_node,
                                          &user_node,
                                          &right_cost);

    double min_cost = 1e38;
    const Node *min_node = NULL;

    assert(beams_[position]->size() > 0);

    if (term_id >= 0) {
      // This token exists in unigram data
      for (int node_id = 0;
           node_id < beams_[position]->size();
           ++node_id) {
        node = beams_[position]->node_at(node_id);
        double cost = CalculateBigramCost(node->term_id,
                                          term_id,
                                          node->cost,
                                          right_cost);
        LOG("Cost: " << cost - node->cost << ", total: " << cost);
        if (cost < min_cost) {
          min_cost = cost;
          min_node = node;
        }
      }

      // Add the min_node to decode graph
      new_node = node_pool_->Alloc();
      new_node->set_value(position + length + 1, term_id, min_cost, min_node);
      beams_[position + length + 1]->Add(new_node);
    } else {
      // One token out-of-vocabulary word should be always put into Decode
      // Graph When no arc to next bucket
      if (length == 0 && beams_[position + 1]->size() == 0) {
        for (int node_id = 0;
             node_id < beams_[position]->size();
             ++node_id) {
          node = beams_[position]->node_at(node_id);
          cost = node->cost + 20;

          if (cost < min_cost) {
            min_cost = cost;
            min_node = node;
          }
        }

        new_node = node_pool_->Alloc();
        new_node->set_value(position + length + 1, 0, min_cost, min_node);
        beams_[position + 1]->Add(new_node);
      }  // end if node count == 0
    }  // end if term_id >= 0

    if (index_flag == false && user_flag == false) break;
  }  // end for length
}

void BigramSegmenter::FindTheBestResult(TermInstance *term_instance, 
                                        TokenInstance *token_instance) {
  // Find the best result from decoding graph
  const Node *node = beams_[token_instance->size()]->MinimalNode();

  // Set the cost data for RecentSegCost()
  cost_ = node->cost;

  term_instance->set_size(node->term_position + 1);
  int beam_id, from_beam_id, term_type;
  std::string buffer;
  while (node->term_position >= 0) {
    buffer.clear();
    beam_id = node->beam_id;
    from_beam_id = node->from_node->beam_id;

    for (int i = from_beam_id; i < beam_id; ++i) {
      buffer.append(token_instance->token_text_at(i));
    }

    term_type = beam_id - from_beam_id > 1?
        Parser::kChineseWord:
        TokenTypeToTermType(token_instance->token_type_at(from_beam_id));

    int oov_id = TermInstance::kTermIdOutOfVocabulary;
    term_instance->set_value_at(
        node->term_position,
        buffer.c_str(),
        beam_id - from_beam_id,
        term_type,
        node->term_id == 0? oov_id: node->term_id);
    node = node->from_node;
  }  // end while
}

void BigramSegmenter::Segment(TermInstance *term_instance,
                              TokenInstance *token_instance) {
  Node *new_node = node_pool_->Alloc();
  new_node->set_value(0, 0, 0, NULL);
  // Add begin-of-sentence node
  beams_[0]->Add(new_node);

  // Strat decoding
  for (int beam_id = 0; beam_id < token_instance->size(); ++beam_id) {
    // Shrink current bucket to ensure node number < n_best
    BuildBeamFromPosition(token_instance, beam_id);
  }  // end for decode_start

  FindTheBestResult(term_instance, token_instance);

  // Clear decode_node
  for (int i = 0; i < token_instance->size() + 1; ++i) {
    beams_[i]->Clear();
  }
  node_pool_->ReleaseAll();
}

}  // namespace milkcat
