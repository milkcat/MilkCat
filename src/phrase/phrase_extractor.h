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
// phrase_extractor.h --- Created at 2014-03-31
//

#ifndef SRC_KEYPHRASE_PHRASE_EXTRACTOR_H_
#define SRC_KEYPHRASE_PHRASE_EXTRACTOR_H_

#include "phrase/document.h"
#include "utils/pool.h"
#include <string>

namespace milkcat {

// In common/trie_tree.h
class TrieTree;

// PhraseExtractor extracts phrases in the document
class PhraseExtractor {
 public:
  class Phrase;

  // Extracts phrases from document and stores into phrases. phrases contains
  // not only multiple word phrases but also the phrases contain only one word.
  // The extracted phrases is allocated by phrase_pool.
  void Extract(const Document *document,
               utils::Pool<Phrase> *phrase_pool,
               std::vector<Phrase *> *phrases);

 private:
  static const double kBoundaryThreshold;
  static const double kPhrasePMIThreshold;
  static const int kFrequencyThreshold = 2;

  class PhraseCandidate;

  const Document *document_;
  std::vector<PhraseCandidate *> from_set_;
  std::vector<PhraseCandidate *> to_set_;
  std::vector<Phrase *> *phrases_;

  utils::Pool<PhraseCandidate> candidate_pool_;
  
  // Calculate the adjacent entropy of the term-frequency map
  inline double CalcAdjent(const utils::unordered_map<int, int> &term_freq);

  // Calculate the left adjacent entropy of the word.
  // @param word_index inversed index of the word
  // @param index_offset offset of each position in word_index
  // @return the entropy
  double Left(const std::vector<int> &word_index, int index_offset);

  // Finds which words occurs most in the right position of word_index
  // @param candidate The candidate phrase
  // @param right_words stores the words occurs most in the right position of 
  //        this word
  double Right(PhraseCandidate *candidate,
               std::vector<int> *right_words);

  // Get a set of words that candidate phrase maybe begin with it, store it into
  // phrases_
  void PhraseBeginSet();

  // Iteration for find phrases from a list of PhraseCandidate specified by
  // from_set_ then release it and put the phrases to phrases and put candidate
  // of phrases into to_set_
  void DoIteration(utils::Pool<Phrase> *phrase_pool);
};

class PhraseExtractor::PhraseCandidate {
 public:
  PhraseCandidate();
  
  // Clears the word and the index of the phrase candidate
  void Clear();

  // Initialize the PhraseCandidate from another PhraseCandidate and the next
  // word
  void FromPhraseCandidateAndWord(
      const Document *document,
      const PhraseCandidate *phrase_candidate,
      int word);

  // Initialize the PhraseCandidate from an index and the first word
  void FromIndexAndWord(const std::vector<int> &index, int word);

  // Returns the string of the PhraseCandidate
  std::string PhraseString(const Document *ducument);

  const std::vector<int> &index() { return index_; }
  const std::vector<int> &phrase_words() { return phrase_words_; }
  int size() const { return phrase_words_.size(); }

  // The sum of log-probability of each word. Using for calculate the pmi
  int sum_logpw() const { return sum_logpw_; }
  void set_sum_logpw(double value) { sum_logpw_ = value; }

  // Weight for the candidate
  int weight() const { return weight_; }
  void set_weight(double value) { weight_ = value; }

 private:
  // Words of this phrase
  std::vector<int> phrase_words_;

  // The index for last word of phrase
  std::vector<int> index_;

  double sum_logpw_;
  double weight_;
};

// A phrase in document
class PhraseExtractor::Phrase {
 public:
  Phrase(): document_(NULL), tf_(0.0), weight_(0.0) {
  }

  void set_document(const Document *document) {
    document_ = document;
  }

  void set_words(const std::vector<int> &words) {
    words_ = words;
  }

  // Final weight for the phrase
  void set_weight(double weight) { weight_ = weight; }
  double weight() const { return weight_; }

  // Gets the string of this phrase
  const char *PhraseString() {
    if (phrase_string_.size() == 0) {
      for (int i = 0; i < words_.size(); ++i)
        phrase_string_ += document_->WordString(words_[i]);
    }

    return phrase_string_.c_str();
  }

  // Word's string at index
  const char *WordString(int index) const {
    assert(index < word_count());
    
    int word = words_[index];
    return document_->WordString(word);
  }

  // Number of words in this phrase
  int word_count() const { return words_.size(); }

 private:
  const Document *document_;
  double tf_;
  double weight_;
  std::vector<int> words_;
  std::string phrase_string_;
};


}  // namespace milkcat

#endif  // SRC_KEYPHRASE_PHRASE_EXTRACTOR_H_