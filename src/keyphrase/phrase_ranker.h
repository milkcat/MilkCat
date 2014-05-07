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
// phrase_ranker.h --- Created at 2014-04-03
//

#ifndef SRC_KEYPHRASE_PHRASE_RANKER_H_
#define SRC_KEYPHRASE_PHRASE_RANKER_H_

#include <vector>
#include "include/milkcat.h"

namespace milkcat {

class Status;
class Document;
class Phrase;
template<typename T> class StringValue;

// PhraseRanker ranks the phrases extracted from document by importance, top N
// will be the keyphrases of the document
class PhraseRanker {
 public:
  PhraseRanker();
  ~PhraseRanker();

  // Creates the PhraseRanker instance. On success, return the pointer to
  // PhraseRanker. On failed, return NULL and set status != Status::OK()
  static PhraseRanker *New(Model::Impl *model_impl, Status *status);

  // Ranks the phrases vector by the weight of phrases
  void Rank(const Document *document, std::vector<Phrase *> *phrases);

 private:
  static const float kDefaultIDF;
  static const double kWeightThreshold;

  const StringValue<float> *idf_model_;

  // Calculate the final score of phrases. Final socre is the combination of
  // each value of the phrase.
  void CalcScore(const Document *document, std::vector<Phrase *> *phrases);

  // Check the weight of the phrase. If the weight larger than kWeightThreshold
  // return false else return true
  static bool NotKeyphrase(Phrase *phrase);
};

}  // namespace milkcat

#endif  // SRC_KEYPHRASE_PHRASE_RANKER_H_
