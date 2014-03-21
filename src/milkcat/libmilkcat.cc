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
// libyaca.cc
// libmilkcat.cc --- Created at 2013-09-03
//

#include "milkcat/libmilkcat.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>
#include "milkcat/bigram_segmenter.h"
#include "milkcat/crf_part_of_speech_tagger.h"
#include "milkcat/crf_tagger.h"
#include "milkcat/hmm_part_of_speech_tagger.h"
#include "milkcat/milkcat.h"
#include "milkcat/mixed_segmenter.h"
#include "milkcat/out_of_vocabulary_word_recognition.h"
#include "milkcat/part_of_speech_tag_instance.h"
#include "milkcat/tokenizer.h"
#include "milkcat/term_instance.h"
#include "milkcat/token_instance.h"
#include "utils/utils.h"

namespace milkcat {

// Model filenames
const char *UNIGRAM_INDEX = "unigram.idx";
const char *UNIGRAM_DATA = "unigram.bin";
const char *BIGRAM_DATA = "bigram.bin";
const char *HMM_PART_OF_SPEECH_MODEL = "ctb_pos.hmm";
const char *CRF_PART_OF_SPEECH_MODEL = "ctb_pos.crf";
const char *CRF_SEGMENTER_MODEL = "ctb_seg.crf";
const char *DEFAULT_TAG = "default_tag.cfg";
const char *OOV_PROPERTY = "oov_property.idx";

Tokenization *TokenizerFactory(int analyzer_type) {
  int tokenizer_type = analyzer_type & kTokenizerMask;

  switch (tokenizer_type) {
    case TOKENIZER_NORMAL:
      return new Tokenization();

    default:
      return NULL;
  }
}

Segmenter *SegmenterFactory(ModelFactory *factory,
                            int analyzer_type,
                            Status *status) {
  int segmenter_type = analyzer_type & kSegmenterMask;

  switch (segmenter_type) {
    case SEGMENTER_BIGRAM:
      return BigramSegmenter::New(factory, true, status);

    case SEGMENTER_UNIGRAM:
      return BigramSegmenter::New(factory, false, status);

    case SEGMENTER_CRF:
      return CRFSegmenter::New(factory, status);

    case SEGMENTER_MIXED:
      return MixedSegmenter::New(factory, status);

    default:
      *status = Status::NotImplemented("Invalid segmenter type");
      return NULL;
  }
}

PartOfSpeechTagger *PartOfSpeechTaggerFactory(ModelFactory *factory,
                                              int analyzer_type,
                                              Status *status) {
  const CRFModel *crf_pos_model;
  int tagger_type = analyzer_type & kPartOfSpeechTaggerMask;

  LOG("Tagger type: %x\n", tagger_type);

  switch (tagger_type) {
    case POSTAGGER_CRF:
      if (status->ok()) crf_pos_model = factory->CRFPosModel(status);

      if (status->ok()) {
        return new CRFPartOfSpeechTagger(crf_pos_model);
      } else {
        return NULL;
      }

    case POSTAGGER_HMM:
      if (status->ok()) {
        return HMMPartOfSpeechTagger::New(factory, false, status);
      } else {
        return NULL;
      }

    case POSTAGGER_MIXED:
      if (status->ok()) {
        return HMMPartOfSpeechTagger::New(factory, true, status);
      } else {
        return NULL;
      }

    case 0:
      return NULL;

    default:
      *status = Status::NotImplemented("Invalid Part-of-speech tagger type");
      return NULL;
  }
}


// The global state saves the current of model and analyzer.
// If any errors occured, global_status != Status::OK()
Status global_status;

// ---------- ModelFactory ----------

ModelFactory::ModelFactory(const char *model_dir_path):
    model_dir_path_(model_dir_path),
    unigram_index_(NULL),
    user_index_(NULL),
    unigram_cost_(NULL),
    user_cost_(NULL),
    bigram_cost_(NULL),
    seg_model_(NULL),
    crf_pos_model_(NULL),
    hmm_pos_model_(NULL),
    oov_property_(NULL) {
}

ModelFactory::~ModelFactory() {
  delete user_index_;
  user_index_ = NULL;

  delete unigram_index_;
  unigram_index_ = NULL;

  delete unigram_cost_;
  unigram_cost_ = NULL;

  delete user_cost_;
  user_cost_ = NULL;

  delete bigram_cost_;
  bigram_cost_ = NULL;

  delete seg_model_;
  seg_model_ = NULL;

  delete crf_pos_model_;
  crf_pos_model_ = NULL;

  delete hmm_pos_model_;
  hmm_pos_model_ = NULL;

  delete oov_property_;
  oov_property_ = NULL;
}

const TrieTree *ModelFactory::Index(Status *status) {
  mutex.Lock();
  if (unigram_index_ == NULL) {
    std::string model_path = model_dir_path_ + UNIGRAM_INDEX;
    unigram_index_ = DoubleArrayTrieTree::New(model_path.c_str(), status);
  }
  mutex.Unlock();
  return unigram_index_;
}

void ModelFactory::LoadUserDictionary(Status *status) {
  char line[1024], word[1024];
  std::string errmsg;
  ReadableFile *fd;
  float default_cost = kDefaultCost, cost;
  std::vector<float> user_costs;
  std::map<std::string, int> term_ids;

  if (user_dictionary_path_ == "") {
    *status = Status::RuntimeError("No user dictionary.");
    return;
  }

  if (status->ok()) fd = ReadableFile::New(user_dictionary_path_.c_str(),
                                           status);
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      char *p = strchr(line, ' ');

      // Checks if the entry has a cost
      if (p != NULL) {
        utils::strlcpy(word, line, p - line + 1);
        utils::trim(word);
        utils::trim(p);
        cost = static_cast<float>(atof(p));
      } else {
        utils::strlcpy(word, line, sizeof(word));
        utils::trim(word);
        cost = default_cost;
      }
      term_ids.insert(std::pair<std::string, int>(
          word, 
          kUserTermIdStart + term_ids.size()));
      user_costs.push_back(cost);
    }
  }

  if (status->ok() && term_ids.size() == 0) {
    errmsg = std::string("User dictionary ") +
             user_dictionary_path_ +
             " is empty.";
    *status = Status::Corruption(errmsg.c_str());
  }


  // Build the index and the cost array from user dictionary
  if (status->ok()) user_index_ = DoubleArrayTrieTree::NewFromMap(term_ids);
  if (status->ok())
    user_cost_ = StaticArray<float>::NewFromArray(user_costs.data(),
                                                  user_costs.size());

  delete fd;
}

const TrieTree *ModelFactory::UserIndex(Status *status) {
  mutex.Lock();
  if (user_index_ == NULL) {
    LoadUserDictionary(status);
  }
  mutex.Unlock();
  return user_index_;
}

const StaticArray<float> *ModelFactory::UserCost(Status *status) {
  mutex.Lock();
  if (user_cost_ == NULL) {
    LoadUserDictionary(status);
  }
  mutex.Unlock();
  return user_cost_;
}

const StaticArray<float> *ModelFactory::UnigramCost(Status *status) {
  mutex.Lock();
  if (unigram_cost_ == NULL) {
    std::string model_path = model_dir_path_ + UNIGRAM_DATA;
    unigram_cost_ = StaticArray<float>::New(model_path.c_str(), status);
  }
  mutex.Unlock();
  return unigram_cost_;
}

const StaticHashTable<int64_t, float> *ModelFactory::BigramCost(
    Status *status) {
  mutex.Lock();
  if (bigram_cost_ == NULL) {
    std::string model_path = model_dir_path_ + BIGRAM_DATA;
    bigram_cost_ = StaticHashTable<int64_t, float>::New(model_path.c_str(),
                                                        status);
  }
  mutex.Unlock();
  return bigram_cost_;
}

const CRFModel *ModelFactory::CRFSegModel(Status *status) {
  mutex.Lock();
  if (seg_model_ == NULL) {
    std::string model_path = model_dir_path_ + CRF_SEGMENTER_MODEL;
    seg_model_ = CRFModel::New(model_path.c_str(), status);
  }
  mutex.Unlock();
  return seg_model_;
}

const CRFModel *ModelFactory::CRFPosModel(Status *status) {
  mutex.Lock();
  if (crf_pos_model_ == NULL) {
    std::string model_path = model_dir_path_ + CRF_PART_OF_SPEECH_MODEL;
    crf_pos_model_ = CRFModel::New(model_path.c_str(), status);
  }
  mutex.Unlock();
  return crf_pos_model_;
}

const HMMModel *ModelFactory::HMMPosModel(Status *status) {
  mutex.Lock();
  if (hmm_pos_model_ == NULL) {
    std::string model_path = model_dir_path_ + HMM_PART_OF_SPEECH_MODEL;
    hmm_pos_model_ = HMMModel::New(model_path.c_str(), status);
  }
  mutex.Unlock();
  return hmm_pos_model_;
}

const TrieTree *ModelFactory::OOVProperty(Status *status) {
  mutex.Lock();
  if (oov_property_ == NULL) {
    std::string model_path = model_dir_path_ + OOV_PROPERTY;
    oov_property_ = DoubleArrayTrieTree::New(model_path.c_str(), status);
  }
  mutex.Unlock();
  return oov_property_;
}

// ---------- Cursor ----------

Cursor::Cursor():
    analyzer_(NULL),
    tokenizer_(TokenizerFactory(TOKENIZER_NORMAL)),
    token_instance_(new TokenInstance()),
    term_instance_(new TermInstance()),
    part_of_speech_tag_instance_(new PartOfSpeechTagInstance()),
    sentence_length_(0),
    current_position_(0),
    end_(0) {
}

Cursor::~Cursor() {
  delete tokenizer_;
  tokenizer_ = NULL;

  delete token_instance_;
  token_instance_ = NULL;

  delete term_instance_;
  term_instance_ = NULL;

  delete part_of_speech_tag_instance_;
  part_of_speech_tag_instance_ = NULL;

  analyzer_ = NULL;
}

void Cursor::Scan(const char *text) {
  tokenizer_->Scan(text);
  sentence_length_ = 0;
  current_position_ = 0;
  end_ = false;
}

void Cursor::MoveToNext() {
  current_position_++;
  if (current_position_ > sentence_length_ - 1) {
    // If reached the end of current sentence

    if (tokenizer_->GetSentence(token_instance_) == false) {
      end_ = true;
    } else {
      analyzer_->segmenter->Segment(term_instance_, token_instance_);

      // If the analyzer have part of speech tagger, tag the term_instance
      if (analyzer_->part_of_speech_tagger) {
        analyzer_->part_of_speech_tagger->Tag(part_of_speech_tag_instance_,
                                              term_instance_);
      }
      sentence_length_ = term_instance_->size();
      current_position_ = 0;
    }
  }
}

}  // namespace milkcat

// ---------- Fucntions in milkcat.h ----------


milkcat_model_t *milkcat_model_new(const char *model_path) {
  if (model_path == NULL) model_path = MODEL_PATH;

  milkcat_model_t *model = new milkcat_model_t;
  model->model_factory = new milkcat::ModelFactory(model_path);

  return model;
}

milkcat_t *milkcat_new(milkcat_model_t *model, int analyzer_type) {
  milkcat::global_status = milkcat::Status::OK();

  milkcat_t *analyzer = new milkcat_t;
  memset(analyzer, 0, sizeof(milkcat_t));

  analyzer->model = model;

  if (milkcat::global_status.ok())
    analyzer->segmenter = milkcat::SegmenterFactory(
        analyzer->model->model_factory,
        analyzer_type,
        &milkcat::global_status);
  if (milkcat::global_status.ok())
    analyzer->part_of_speech_tagger = milkcat::PartOfSpeechTaggerFactory(
        analyzer->model->model_factory,
        analyzer_type,
        &milkcat::global_status);

  if (!milkcat::global_status.ok()) {
    milkcat_destroy(analyzer);
    return NULL;
  } else {
    return analyzer;
  }
}

void milkcat_model_destroy(milkcat_model_t *model) {
  if (model == NULL) return;

  delete model->model_factory;
  model->model_factory = NULL;

  delete model;
}

void milkcat_destroy(milkcat_t *analyzer) {
  if (analyzer == NULL) return;

  analyzer->model = NULL;

  delete analyzer->segmenter;
  analyzer->segmenter = NULL;

  delete analyzer->part_of_speech_tagger;
  analyzer->part_of_speech_tagger = NULL;

  delete analyzer;
}

void milkcat_model_set_userdict(milkcat_model_t *model, const char *path) {
  model->model_factory->SetUserDictionary(path);
}

void milkcat_analyze(milkcat_t *analyzer, 
                     milkcat_cursor_t *cursor,
                     const char *text) {
  milkcat::Cursor *internal_cursor = cursor->internal_cursor;

  internal_cursor->set_analyzer(analyzer);
  internal_cursor->Scan(text);
}

int milkcat_cursor_get_next(milkcat_cursor_t *cursor,
                            milkcat_item_t *next_item) {
  milkcat::Cursor *internal_cursor = cursor->internal_cursor;

  // If the cursor has not used or already reaches the end
  if (internal_cursor->analyzer() == NULL) return MC_NONE;

  internal_cursor->MoveToNext();

  // If reached the end of text, collect back the cursor then return
  // MC_NONE
  if (internal_cursor->end()) return MC_NONE;

  next_item->word = internal_cursor->word();
  next_item->part_of_speech_tag = internal_cursor->part_of_speech_tag();
  next_item->word_type = internal_cursor->word_type();

  return MC_OK;
}

milkcat_cursor_t *milkcat_cursor_new() {
  milkcat_cursor_t *cursor = new milkcat_cursor_t;
  cursor->internal_cursor = new milkcat::Cursor();
  return cursor;
}

void milkcat_cursor_destroy(milkcat_cursor_t *cursor) {
  if (cursor == NULL) return;

  delete cursor->internal_cursor;
  cursor->internal_cursor = NULL;

  delete cursor;
}

const char *milkcat_last_error() {
  return milkcat::global_status.what();
}

