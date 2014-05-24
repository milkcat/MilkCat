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

#include "keyphrase/phrase_extractor.h"

#include <math.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "keyphrase/string_table.h"
#include "keyphrase/document.h"
#include "utils/log.h"
#include "utils/utils.h"

namespace milkcat {

const double PhraseExtractor::kShiftThreshold = 2.0;
const double PhraseExtractor::kBoundaryThreshold = 0.5;

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

inline void PhraseExtractor::CalcAdjacent(
    const utils::unordered_map<int, int> &term_freq,
    Adjacent *adjacent) {
  int sum = 0;
  int max_freq = 0;
  int max_word = 0;
  for (utils::unordered_map<int, int>::const_iterator
       it = term_freq.begin(); it != term_freq.end(); ++it) {
    sum += it->second;
    if (it->second > max_freq) {
      max_freq = it->second;
      max_word = it->first;
    }
  }

  adjacent->entropy = 0.0;
  double p;
  for (utils::unordered_map<int, int>::const_iterator
       it = term_freq.begin(); it != term_freq.end(); ++it) {
    p = it->second / (1e-38 + sum);
    adjacent->entropy += -p * log(p);
  }

  adjacent->major_word = max_word;
  adjacent->major_word_freq = max_freq;
}

void PhraseExtractor::LeftAdjacent(const std::vector<int> &word_index,
                                   int index_offset,
                                   Adjacent *adjacent) {
  utils::unordered_map<int, int> term_freq;

  for (int i = 0; i < word_index.size(); ++i) {
    int idx = word_index.at(i);
    if (idx > 0) {
      int left_term = document_->word(idx - index_offset - 1);
      term_freq[left_term] += 1;
    }
  }

  CalcAdjacent(term_freq, adjacent);
}

void PhraseExtractor::RightAdjacent(const std::vector<int> &word_index,
                                    Adjacent *adjacent) {
  utils::unordered_map<int, int> term_freq;

  for (int i = 0; i < word_index.size(); ++i) {
    int idx = word_index.at(i);
    if (idx != document_->size() - 1) {
      int right_word = document_->word(idx + 1);
      term_freq[right_word] += 1;
    }
  }

  CalcAdjacent(term_freq, adjacent);
}

// Get a set of terms that candidate phrase maybe begin with it
void PhraseExtractor::PhraseBeginSet() {
  Adjacent adjacent;

  for (int word = 0; word < document_->words_size(); ++word) {
    const std::vector<int> &index = document_->word_index(word);
    if (document_->tf(word) > 1 && document_->is_stopword(word) == false) {
      LeftAdjacent(index, 0, &adjacent);

      if (IsBoundary(adjacent)) {
        PhraseCandidate *phrase_candidate = candidate_pool_.Alloc();
        phrase_candidate->FromIndexAndWord(index, word);
        from_set_.push_back(phrase_candidate);
      }
    }
  }
}

void PhraseExtractor::DoIteration(utils::Pool<Phrase> *phrase_pool) {
  Adjacent adjacent;

  while (from_set_.size()) {
    PhraseCandidate *from_phrase = from_set_.back();
    from_set_.pop_back();

    // Get the right adjacent data and check if it is the boundary of phrase
    RightAdjacent(from_phrase->index(), &adjacent);

    if (!document_->is_stopword(adjacent.major_word) &&
        document_->tf(adjacent.major_word) > 2 &&
        IsPhrase(adjacent)) {
      // Create the new candidate of phrase
      PhraseCandidate *to_phrase = candidate_pool_.Alloc();
      to_phrase->FromPhraseCandidateAndWord(document_,
                                            from_phrase,
                                            adjacent.major_word);
      to_set_.push_back(to_phrase);    
    } 

    if (IsBoundary(adjacent)) {
      // Recalculate the Left Adjacent Entropy
      LeftAdjacent(from_phrase->index(),
                   from_phrase->phrase_words().size() - 1,
                   &adjacent);
 
      // Ensure adjacent entropy of every phrase larger than kBoundaryThreshold
      if (from_phrase->index().size() > 1 && 
          adjacent.entropy > kBoundaryThreshold) {
        Phrase *phrase = phrase_pool->Alloc();
        phrase->set_document(document_);
        phrase->set_words(from_phrase->phrase_words());
        double tf = from_phrase->index().size() / (1e-38 + document_->size());
        phrase->set_tf(tf);
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
    DoIteration(phrase_pool);
    from_set_.clear();
    from_set_.swap(to_set_);
  }
}

}  // namespace milkcat
