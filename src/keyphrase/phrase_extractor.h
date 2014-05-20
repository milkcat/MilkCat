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

#include "keyphrase/document.h"
#include "keyphrase/phrase.h"
#include "utils/pool.h"
#include <string>

namespace milkcat {

// In common/trie_tree.h
class TrieTree;

// PhraseExtractor extracts phrases in the document
class PhraseExtractor {
 public:
  // Extracts phrases from document and stores into phrases. phrases contains
  // not only multiple word phrases but also the phrases contain only one word.
  // The extracted phrases is allocated by phrase_pool.
  void Extract(const Document *document,
               utils::Pool<Phrase> *phrase_pool,
               std::vector<Phrase *> *phrases);

 private:
  static const double kBoundaryThreshold;
  static const double kShiftThreshold;

  // Adjacent represent the adjacent data for a phrase contains the adjacent
  // entropy which word occurs or which word occurs most 
  struct Adjacent {
    int major_word;
    int major_word_freq;
    double entropy;
  };

  // This structure represent a candidate of phrase. It stores the words
  // contained by this phrase and the phrase's index in document
  class PhraseCandidate;

  const Document *document_;
  std::vector<PhraseCandidate *> from_set_;
  std::vector<PhraseCandidate *> to_set_;
  std::vector<Phrase *> *phrases_;

  utils::Pool<PhraseCandidate> candidate_pool_;
  
  // Calculate the major term id and its term frequency as well as adjacent
  // entropy from an unordered_map contained term frequency data
  inline void CalcAdjacent(const utils::unordered_map<int, int> &term_freq,
                           Adjacent *adjacent);

  // Finds which word occurs most in the left position of word_index,
  // returns the word and the entrypy. index_offset is the offset in the
  // word_index
  void LeftAdjacent(const std::vector<int> &word_index,
                    int index_offset,
                    Adjacent *adjacent);

  // Finds which word occurs most in the right position of word_index,
  // returns the word and the entrypy.
  void RightAdjacent(const std::vector<int> &word_index,
                     Adjacent *adjacent);

  // Whether the next term is part of current phrase
  inline bool IsPhrase(const Adjacent &adjacent) {
    return adjacent.major_word_freq > 1 && adjacent.entropy < kShiftThreshold;
  }

  // Whether the left or right term is the boundary of a term
  inline bool IsBoundary(const Adjacent &adjacent) {
    return adjacent.entropy > kBoundaryThreshold;
  }

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

 private:
  // Words of this phrase
  std::vector<int> phrase_words_;

  // The index for last word of phrase
  std::vector<int> index_;
};

}  // namespace milkcat

#endif  // SRC_KEYPHRASE_PHRASE_EXTRACTOR_H_