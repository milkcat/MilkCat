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
// model_factory.cc --- Created at 2014-04-02
// model_impl.cc --- Created at 2014-09-02
//

#include "common/model_impl.h"

#include "libmilkcat.h"
#include "ml/perceptron_model.h"
#include "common/milkcat_config.h"
#include "common/reimu_trie.h"
#include "common/static_array.h"
#include "common/static_hashtable.h"
#include "ml/crf_model.h"
#include "ml/hmm_model.h"
#include "parser/feature_template.h"

namespace milkcat {

// Model filenames
const char *kUnigramIndexFile = "unigram.idx";
const char *kUnigramDataFile = "unigram.bin";
const char *kBigramDataFile = "bigram.bin";
const char *kHmmPosModelFile = "ctb_pos.hmm";
const char *kCrfPosModelFile = "ctb_pos.crf";
const char *kCrfSegModelFile = "ctb_seg.crf";
const char *kOovPropertyFile = "oov_property.idx";
const char *kStopwordFile = "stopword.idx";
const char *kBeamYamadaModelPrefix = "ctb_dep.b8";
const char *kYamadaModelPrefix = "ctb_dep.b1";
const char *kDependenctTemplateFile = "depparse.tmpl";

// ---------- Model::Impl ----------

Model::Model(const char *model_dir):
    model_dir_(model_dir),
    unigram_index_(NULL),
    user_index_(NULL),
    unigram_cost_(NULL),
    user_cost_(NULL),
    bigram_cost_(NULL),
    seg_model_(NULL),
    crf_pos_model_(NULL),
    hmm_pos_model_(NULL),
    oov_property_(NULL),
    dependency_(NULL),
    dependency_feature_(NULL) {
  if (model_dir_.back() != '/' && model_dir_.back() != '\\') {
    model_dir_.push_back('/');
  }
}

Model::~Model() {
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

  delete dependency_;
  dependency_ = NULL;

  delete dependency_feature_;
  dependency_feature_ = NULL;
}

const ReimuTrie *Model::Index(Status *status) {
  if (unigram_index_ == NULL) {
    std::string model_path = model_dir_ + kUnigramIndexFile;
    unigram_index_ = ReimuTrie::Open(model_path.c_str());
    if (unigram_index_ == NULL) {
      std::string errmsg = "Unable to open ";
      errmsg += model_path;
      *status = Status::IOError(errmsg.c_str());
    }
  }
  return unigram_index_;
}

void Model::ReadUserDictionary(const char *path, Status *status) {
  char line[1024], word[1024], cost_string[1024];
  std::string errmsg;
  ReadableFile *fd = NULL;
  float default_cost = kDefaultCost, cost;
  std::vector<float> user_cost;
  ReimuTrie *user_index = new ReimuTrie();

  if (status->ok()) fd = ReadableFile::New(path, status);
  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    
    if (status->ok()) {
      // Ignore empty line
      trim(line);
      if (*line == '\0') continue;

      char *p = strchr(line, ' ');
      // Checks if the entry has a cost
      if (p != NULL) {
        strlcpy(word, line, p - line + 1);
        strlcpy(cost_string, p, sizeof(cost_string));
        trim(word);
        trim(cost_string);
        char *end = NULL;
        cost = strtof(cost_string, &end);
        // If second field is not a valid float number
        if (*end != '\0') {
          std::string errmsg = "unexpected cost field: ";
          errmsg += line;
          *status = Status::RuntimeError(errmsg.c_str());
        }
      } else {
        strlcpy(word, line, sizeof(word));
        trim(word);
        cost = default_cost;
      }
      user_index->Put(word, kUserTermIdStart + user_cost.size());
      user_cost.push_back(cost);
    }
  }

  if (status->ok()) {
    delete user_cost_;
    user_index_ = user_index;
    user_cost_ = StaticArray<float>::NewFromArray(user_cost.data(),
                                                  user_cost.size());
  }

  delete fd;
}

const ReimuTrie *Model::UserIndex(Status *status) {
  if (user_index_) {
    return user_index_;
  } else {
    *status = Status::RuntimeError("No user dictionary");
    return NULL;
  }
}

const StaticArray<float> *Model::UserCost(Status *status) {
  if (user_cost_) {
    return user_cost_;
  } else {
    *status = Status::RuntimeError("No user dictionary");
    return NULL;
  }
}

const StaticArray<float> *Model::UnigramCost(Status *status) {
  if (unigram_cost_ == NULL) {
    std::string model_path = model_dir_ + kUnigramDataFile;
    unigram_cost_ = StaticArray<float>::New(model_path.c_str(), status);
  }
  return unigram_cost_;
}

const StaticHashTable<int64_t, float> *Model::BigramCost(Status *status) {
  if (bigram_cost_ == NULL) {
    std::string model_path = model_dir_ + kBigramDataFile;
    bigram_cost_ = StaticHashTable<int64_t, float>::New(model_path.c_str(),
                                                        status);
  }
  return bigram_cost_;
}

const CRFModel *Model::CRFSegModel(Status *status) {
  if (seg_model_ == NULL) {
    std::string model_path = model_dir_ + kCrfSegModelFile;
    seg_model_ = CRFModel::New(model_path.c_str(), status);
  }
  return seg_model_;
}

const CRFModel *Model::CRFPosModel(Status *status) {
  if (crf_pos_model_ == NULL) {
    std::string model_path = model_dir_ + kCrfPosModelFile;
    crf_pos_model_ = CRFModel::New(model_path.c_str(), status);
  }
  return crf_pos_model_;
}

const HMMModel *Model::HMMPosModel(Status *status) {
  if (hmm_pos_model_ == NULL) {
    std::string model_path = model_dir_ + kHmmPosModelFile;
    hmm_pos_model_ = HMMModel::New(model_path.c_str(), status);
  }
  return hmm_pos_model_;
}

const ReimuTrie *Model::OOVProperty(Status *status) {
  if (oov_property_ == NULL) {
    std::string model_path = model_dir_ + kOovPropertyFile;
    oov_property_ = ReimuTrie::Open(model_path.c_str());
    if (oov_property_ == NULL) {
      std::string errmsg = "Unable to open out-of-vocabulary property file: ";
      errmsg += model_path;
      *status = Status::IOError(errmsg.c_str());
    }
  }
  return oov_property_;
}

PerceptronModel *Model::YamadaModel(Status *status) {
  if (dependency_ == NULL) {
    std::string prefix = model_dir_ + kYamadaModelPrefix;
    dependency_ = PerceptronModel::Open(prefix.c_str(), status);
  }
  return dependency_;  
}

PerceptronModel *Model::BeamYamadaModel(Status *status) {
  if (dependency_ == NULL) {
    std::string prefix = model_dir_ + kBeamYamadaModelPrefix;
    dependency_ = PerceptronModel::Open(prefix.c_str(), status);
  }
  return dependency_;  
}

DependencyParser::FeatureTemplate *
Model::DependencyTemplate(Status *status) {
  if (dependency_feature_ == NULL) {
    std::string prefix = model_dir_ + kDependenctTemplateFile;
    dependency_feature_ = DependencyParser::FeatureTemplate::Open(
        prefix.c_str(),
        status);
  }
  return dependency_feature_;  
}

}  // namespace milkcat
