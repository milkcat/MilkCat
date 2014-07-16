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
// candidate_term.cc --- Created at 2014-03-26
//

#define ENABLE_LOG

#include "phrase/phrase_extractor.h"

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
    weight_(0.0) {
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
}

void PhraseExtractor::PhraseCandidate::FromIndexAndWord(
    const std::vector<int> &index,
    int word) {
  Clear();
  phrase_words_.push_back(word);
  index_ = index;
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
        phrase_candidate->FromIndexAndWord(index, word);
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
      adjacent = Left(from_phrase->index(),
                      from_phrase->phrase_words().size() - 1);
 
      // Ensure adjacent entropy of every phrase larger than kBoundaryThreshold
      if (from_phrase->index().size() > 1 && 
          adjacent > kBoundaryThreshold) {
        Phrase *phrase = phrase_pool->Alloc();
        phrase->set_document(document_);
        phrase->set_words(from_phrase->phrase_words());
        phrases_->push_back(phrase);
      }
    }
  }
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
    LOG("from_set = " << from_set_.size());
    LOG("to_set = " << to_set_.size());
    from_set_.clear();
    from_set_.swap(to_set_);
  }
}

}  // namespace milkcat
