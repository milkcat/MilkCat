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
// candidate_term.cc --- Created at 2014-03-26
//

#define DEBUG

#include "phrase/phrase_extractor.h"

#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "phrase/string_table.h"
#include "phrase/document.h"
#include "utils/log.h"
#include "utils/utils.h"

namespace milkcat {

const double PhraseExtractor::kPhrasePMIThreshold = 10.0;
const double PhraseExtractor::kBoundaryThreshold = 0.5;

PhraseExtractor::PhraseCandidate::PhraseCandidate(): 
    sum_logpw_(0.0),
    pmi_(0.0) {
}

void PhraseExtractor::PhraseCandidate::Clear() {
  phrase_words_.clear();
  index_.clear();
}

void PhraseExtractor::PhraseCandidate::FromPhraseCandidateAndWord(
    const Document *document,
    const PhraseCandidate *from,
    int word) {
  Clear();
  phrase_words_ = from->phrase_words_;
  phrase_words_.push_back(word);

  // OK, make the new inversed_index
  for (std::vector<int>::const_iterator
       it = from->index_.begin(); it != from->index_.end(); ++it) {
    int idx = *it;

    // If the word is not the end of document
    if (idx < document->size() - 1) {
      if (document->word(idx + 1) == word) index_.push_back(idx + 1);
    }
  }

  // Update PMI values
  sum_logpw_ = from->sum_logpw() + log(document->tf(word));
  double phrase_freq = static_cast<double>(index_.size()) / document->size();
  pmi_ = log(phrase_freq) - sum_logpw_;
  LOG("pmi_ = ", pmi_);
}

void PhraseExtractor::PhraseCandidate::FromIndexAndWord(
    const Document *document,
    const std::vector<int> &index,
    int word) {
  Clear();
  phrase_words_.push_back(word);
  index_ = index;

  // Update PMI values
  sum_logpw_ = log(document->tf(word));
}

std::string PhraseExtractor::PhraseCandidate::PhraseString(
    const Document *document) {
  std::string phrase_string;
  for (std::vector<int>::iterator
       it = phrase_words_.begin(); it != phrase_words_.end(); ++it) {
    phrase_string += document->WordString(*it);
  }

  return phrase_string;
}

inline double PhraseExtractor::CalcAdjent(
    const utils::unordered_map<int, int> &term_freq) {
  int sum = 0;
  int max_freq = 0;
  int max_word = 0;

  for (utils::unordered_map<int, int>::const_iterator
       it = term_freq.begin(); it != term_freq.end(); ++it) {
    sum += it->second;
  }

  double entropy = 0.0;
  double p;
  for (utils::unordered_map<int, int>::const_iterator
       it = term_freq.begin(); it != term_freq.end(); ++it) {
    p = it->second / (1e-38 + sum);
    entropy += -p * log(p);
  }

  return entropy;
}

double PhraseExtractor::Left(const std::vector<int> &word_index,
                             int index_offset) {
  utils::unordered_map<int, int> term_freq;

  for (int i = 0; i < word_index.size(); ++i) {
    int idx = word_index.at(i);
    if (idx > 0) {
      int left_term = document_->word(idx - index_offset - 1);
      term_freq[left_term] += 1;
    }
  }

  return CalcAdjent(term_freq);
}

template <typename T>
bool CompareItemsByValue(const T &it1, const T &it2) {
  return it1.second < it2.second;
} 

double PhraseExtractor::Right(PhraseExtractor::PhraseCandidate *candidate,
                              std::vector<int> *right_words) {
  utils::unordered_map<int, int> term_freq;

  for (int i = 0; i < candidate->index().size(); ++i) {
    int idx = candidate->index().at(i);
    if (idx != document_->size() - 1) {
      int right_word = document_->word(idx + 1);
      term_freq[right_word] += 1;
    }
  }

  double entropy = CalcAdjent(term_freq);

  // Find which word in the right will constructs a phrase
  right_words->clear();
  for (utils::unordered_map<int, int>::iterator
       it = term_freq.begin(); it != term_freq.end(); ++it) {
    int word = it->first;
    int right_freq = it->second;

    if (right_freq < kFrequencyThreshold) continue;
    if (document_->is_stopword(word)) continue;

    // Calculate Pointwise Mutual Information
    double p_left = static_cast<double>(
        candidate->index().size()) / document_->size();
    double p_right = document_->tf(word);

    double p_joint = static_cast<double>(right_freq) / document_->size();
    double pmi = log(p_joint) - log(p_left) - log(p_right);
    
    // LOG("PMI of " << candidate->PhraseString(document_).c_str() << ", "
    //     << document_->WordString(word) << ": " << pmi);
    right_words->push_back(word);
  }

  return entropy;
}

// Get a set of terms that candidate phrase maybe begin with it
void PhraseExtractor::PhraseBeginSet() {
  double adjacent = 0.0;

  for (int word = 0; word < document_->words_size(); ++word) {
    const std::vector<int> &index = document_->word_index(word);
    if (document_->word_index(word).size() > 1 && 
        document_->is_stopword(word) == false) {
      adjacent = Left(index, 0);

      if (adjacent > kBoundaryThreshold) {
        PhraseCandidate *phrase_candidate = candidate_pool_.Alloc();
        phrase_candidate->FromIndexAndWord(document_, index, word);
        from_set_.push_back(phrase_candidate);
      }
    }
  }
}

void PhraseExtractor::DoIteration(utils::Pool<Phrase> *phrase_pool) {
  double adjacent;
  std::vector<int> right_words;

  while (from_set_.size()) {
    PhraseCandidate *from_phrase = from_set_.back();
    from_set_.pop_back();

    // Get the right adjacent data and check if it is the boundary of phrase
    adjacent = Right(from_phrase, &right_words);

    for (std::vector<int>::iterator
         it = right_words.begin(); it != right_words.end(); ++it) {
      int right_word = *it;
      

      PhraseCandidate *to_phrase = candidate_pool_.Alloc();
      to_phrase->FromPhraseCandidateAndWord(document_,
                                            from_phrase,
                                            right_word);
      to_set_.push_back(to_phrase);
    }

    // If current word is the end of phrase
    if (adjacent > kBoundaryThreshold && from_phrase->size() > 1) {
      // Recalculate the Left Adjacent Entropy
      double left_adjacent = Left(
          from_phrase->index(),
          from_phrase->phrase_words().size() - 1);
 
      // Ensure adjacent entropy of every phrase larger than kBoundaryThreshold
      if (from_phrase->index().size() > 1 && 
          adjacent > kBoundaryThreshold) {
        Phrase *phrase = phrase_pool->Alloc();
        phrase->set_document(document_);
        phrase->set_words(from_phrase->phrase_words());
        phrase->set_PMI(from_phrase->PMI());
        phrase->set_adjent(std::min(left_adjacent, adjacent));
        phrases_->push_back(phrase);
      }
    }
  }
}

void PhraseExtractor::CalculateWeight(std::vector<Phrase *> *phrases) {
  // Calculate the weight of each phrase. Assuming that PMI and AdjEnt are
  // normally distributed.
  // Calculate mean
  double PMI_sum = 0.0;
  double adjent_sum = 0.0;
  for (std::vector<Phrase *>::iterator
       it = phrases->begin(); it != phrases->end(); ++it) {
    PMI_sum += (*it)->PMI();
    adjent_sum += (*it)->adjent();
  }
  double PMI_mean = PMI_sum / phrases->size();
  double adjent_mean = adjent_sum / phrases->size();
  LOG("PMI_mean = ", PMI_mean);
  LOG("adjent_mean = ", adjent_mean);

  // Calculate variance
  double PMI_variance = 0.0;
  double adjent_variance = 0.0;
  for (std::vector<Phrase *>::iterator
       it = phrases->begin(); it != phrases->end(); ++it) {
    PMI_variance += pow((*it)->PMI() - PMI_mean, 2);
    adjent_variance += pow((*it)->adjent() - adjent_mean, 2);
  }
  double PMI_sigma = sqrt(PMI_variance / phrases->size());
  double adjent_sigma = sqrt(adjent_variance / phrases->size());
  LOG("PMI_sigma = ", PMI_sigma);
  LOG("adjent_sigma = ", adjent_sigma);

  // Calcuate weight
  for (std::vector<Phrase *>::iterator
       it = phrases->begin(); it != phrases->end(); ++it) {
    double PMI_normalized = ((*it)->PMI() - PMI_mean) / PMI_sigma;
    double adjent_normalized = ((*it)->adjent() - adjent_mean) / adjent_sigma;
    PMI_normalized = std::min(PMI_normalized, 3.0);
    adjent_normalized = std::min(adjent_normalized, 3.0);
    (*it)->set_PMI(PMI_normalized);
    (*it)->set_adjent(adjent_normalized);
    (*it)->set_weight(adjent_normalized + PMI_normalized);
    LOG((*it)->PhraseString(), " ", PMI_normalized, adjent_normalized);
  }
}

// Remove the phrase if either PMI or adjent less than 0. Note that the values
// are normalized during CalculateWeight
inline bool
PhraseFilter(const PhraseExtractor::Phrase *phrase) {
  LOG("PhraseFilter ", phrase->PMI(), " ",
      phrase->adjent(), " ", phrase->PMI() < 0.0 || phrase->adjent() < 0.0);
  return phrase->PMI() < 0.0 || phrase->adjent() < 0.0;
}

// Compares two phrases by its weight
inline bool
WeightCompare(const PhraseExtractor::Phrase *phrase,
              const PhraseExtractor::Phrase *phrase2) {
  return phrase->weight() < phrase2->weight();
}

void PhraseExtractor::Extract(const Document *document,
                              utils::Pool<Phrase> *phrase_pool,
                              std::vector<Phrase *> *phrases) {
  document_ = document;
  phrases_ = phrases;

  // Clean the previous data
  candidate_pool_.ReleaseAll();
  phrase_pool->ReleaseAll();
  from_set_.clear();
  to_set_.clear();
  phrases_->clear();

  // Get the phrase candidates' begin set 
  PhraseBeginSet();

  // Extract the phrases until the size of from_set reaches 0
  while (from_set_.size() != 0) {
    LOG("DoIteration");
    DoIteration(phrase_pool);
    LOG("from_set = ", from_set_.size());
    LOG("to_set = ", to_set_.size());
    from_set_.clear();
    from_set_.swap(to_set_);
  }

  CalculateWeight(phrases);
  phrases->erase(
      std::remove_if(phrases->begin(), phrases->end(), PhraseFilter),
      phrases->end());
  std::sort(phrases->begin(), phrases->end(), WeightCompare);
}

}  // namespace milkcat
