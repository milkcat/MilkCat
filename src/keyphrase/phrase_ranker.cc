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
// phrase_ranker.cc --- Created at 2014-04-04
//

// #define ENABLE_LOG

#include "keyphrase/phrase_ranker.h"

#include <math.h>
#include <string.h>
#include <algorithm>
#include "common/model_factory.h"
#include "keyphrase/string_value.h"
#include "keyphrase/phrase.h"

namespace milkcat {

PhraseRanker::PhraseRanker(): idf_model_(NULL) {
}

PhraseRanker::~PhraseRanker() {
}

PhraseRanker *PhraseRanker::New(ModelFactory *model_factory, Status *status) {
  PhraseRanker *self = new PhraseRanker();
  self->idf_model_ = model_factory->IDFModel(status);

  if (status->ok()) {
    return self;
  } else {
    delete self;
    return NULL;
  }
}

// Compare two phrase pointers by the weight of the phrase it points
inline bool PhraseScoreCmp(const Phrase *phrase1, const Phrase *phrase2) {
  return phrase1->weight() > phrase2->weight();
}

inline bool PhraseRanker::NotKeyphrase(Phrase *phrase) {
  return phrase->weight() < kWeightThreshold;
}

void PhraseRanker::Rank(const Document &document,
                        std::vector<Phrase *> *phrases) {
  CalcScore(document, phrases);
  
  phrases->erase(
      std::remove_if(phrases->begin(), phrases->end(), NotKeyphrase),
      phrases->end());
  std::sort(phrases->begin(), phrases->end(), PhraseScoreCmp);
}

const float PhraseRanker::kDefaultIDF = 8.0f;
const double PhraseRanker::kWeightThreshold = 0.01;
void PhraseRanker::CalcScore(const Document &document,
                             std::vector<Phrase *> *phrases) {
  double tfidf_sum, alpha;
  float idf;
  double max_value = 0.0;

  for (std::vector<Phrase *>::iterator
       it = phrases->begin(); it != phrases->end(); ++it) {
    Phrase *phrase = *it;
    int word_num = phrase->word_count();
 
    tfidf_sum = 0.0;
    for (int i = 0; i < word_num; ++i) {
      if (!idf_model_->Get(phrase->WordString(i), &idf))
        idf = kDefaultIDF;

      tfidf_sum += phrase->tf() * (idf + 2.0 * std::max(0, word_num - 1));
    }

    phrase->set_weight(tfidf_sum);
  }
}

}  // namespace milkcat