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
// mkgram.cc --- Created at 2013-10-21
// (with) mkdict.cc --- Created at 2013-06-08
// mk_model.cc -- Created at 2013-11-08
// mctools.cc -- Created at 2014-02-21
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <map>
#include <string>
#include <algorithm>
#include <set>
#include "common/darts.h"
#include "common/static_hashtable.h"
#include "common/trie_tree.h"
#include "ml/multiclass_perceptron_model.h"
#include "include/milkcat.h"
#include "parser/beam_arceager_dependency_parser.h"
#include "parser/dependency_parser.h"
#include "parser/naive_arceager_dependency_parser.h"
#include "tagger/hmm_part_of_speech_tagger.h"
#include "utils/utils.h"
#include "utils/readable_file.h"
#include "utils/writable_file.h"

namespace milkcat {

#pragma pack(1)
struct BigramRecord {
  int32_t word_left;
  int32_t word_right;
  float weight;
};

struct HMMEmitRecord {
  int32_t term_id;
  int32_t tag_id;
  float weight;
};
#pragma pack(0)

#define UNIGRAM_INDEX_FILE "unigram.idx"
#define UNIGRAM_DATA_FILE "unigram.bin"
#define BIGRAM_FILE "bigram.bin"
#define HMM_MODEL_FILE "hmm_model.bin"

// Load unigram data from unigram_file, if an error occured set status !=
// Status::OK()
void LoadUnigramFile(const char *unigram_file,
                     std::map<std::string, double> *unigram_data,
                     Status *status) {
  ReadableFile *fd = ReadableFile::New(unigram_file, status);
  char word[1024], line[1024];
  double count, sum = 0;

  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      sscanf(line, "%s %lf", word, &count);
      (*unigram_data)[std::string(word)] += count;
      sum += count;
    }
  }

  // Calculate the weight = -log(freq / total)
  if (status->ok()) {
    for (std::map<std::string, double>::iterator
         it = unigram_data->begin(); it != unigram_data->end(); ++it) {
      it->second = -log(it->second / sum);
    }
  }

  delete fd;
}

// Load bigram data from bigram_file, if an error occured set status !=
// Status::OK()
void LoadBigramFile(const char *bigram_file,
                    std::map<std::pair<std::string, std::string>, int> *bigram_data,
                    int *total_count,
                    Status *status) {
  char left[100], right[100], line[1024];
  int count;

  ReadableFile *fd = ReadableFile::New(bigram_file, status);
  *total_count = 0;

  while (status->ok() && !fd->Eof()) {
    fd->ReadLine(line, sizeof(line), status);
    if (status->ok()) {
      sscanf(line, "%s %s %d", left, right, &count);
      (*bigram_data)[std::pair<std::string, std::string>(left, right)] += count;
      *total_count += count;
    }
  }

  delete fd;
}

// Build Double-Array TrieTree index from unigram, and save the index and the
// unigram data file
void BuildAndSaveUnigramData(const std::map<std::string, double> &unigram_data,
                             Darts::DoubleArray *double_array,
                             Status *status) {
  std::vector<const char *> key;
  std::vector<Darts::DoubleArray::value_type> term_id;
  std::vector<float> weight;

  // term_id = 0 is reserved for out-of-vocabulary word
  int i = 1;
  weight.push_back(0.0);

  for (std::map<std::string, double>::const_iterator
       it = unigram_data.begin(); it != unigram_data.end(); ++it) {
    key.push_back(it->first.c_str());
    term_id.push_back(i++);
    weight.push_back(it->second);
  }

  int result = double_array->build(key.size(), &key[0], 0, &term_id[0]);
  if (result != 0) *status = Status::RuntimeError("unable to build trie-tree");

  WritableFile *fd = NULL;
  if (status->ok()) fd = WritableFile::New(UNIGRAM_DATA_FILE, status);
  if (status->ok())
    fd->Write(weight.data(), sizeof(float) * weight.size(), status);


  if (status->ok()) {
    if (0 != double_array->save(UNIGRAM_INDEX_FILE)) {
      std::string message = "unable to save index file ";
      message += UNIGRAM_INDEX_FILE;
      *status = Status::RuntimeError(message.c_str());
    }
  }

  delete fd;
}

// Save unigram data into binary file UNIGRAM_FILE. On success, return the
// number of bigram word pairs successfully writed. On failed, set status !=
// Status::OK()
int SaveBigramBinFile(
    const std::map<std::pair<std::string, std::string>, int> &bigram_data,
    int total_count,
    const Darts::DoubleArray &double_array,
    Status *status) {
  const char *left_word, *right_word;
  int32_t left_id, right_id;
  int count;
  std::vector<int64_t> keys;
  std::vector<float> values;

  for (std::map<std::pair<std::string, std::string>, int>::const_iterator
       it = bigram_data.begin(); it != bigram_data.end(); ++it) {
    left_word = it->first.first.c_str();
    right_word = it->first.second.c_str();
    count = it->second;
    left_id = double_array.exactMatchSearch<int>(left_word);
    right_id = double_array.exactMatchSearch<int>(right_word);
    if (left_id > 0 && right_id > 0) {
      keys.push_back((static_cast<int64_t>(left_id) << 32) + right_id);
      values.push_back(-log(static_cast<double>(count) / total_count));
    }
  }

  const StaticHashTable<int64_t, float> *
  hashtable = StaticHashTable<int64_t, float>::Build(
      keys.data(),
      values.data(),
      keys.size());
  hashtable->Save(BIGRAM_FILE, status);

  delete hashtable;
  return keys.size();
}

int MakeGramModel(int argc, char **argv) {
  Darts::DoubleArray double_array;
  std::map<std::string, double> unigram_data;
  std::map<std::pair<std::string, std::string>, int> bigram_data;
  Status status;

  if (argc != 4)
    status = Status::Info("Usage: mc_model gram [UNIGRAM FILE] [BIGRAM FILE]");

  const char *unigram_file = argv[argc - 2];
  const char *bigram_file = argv[argc - 1];

  if (status.ok()) {
    printf("Loading unigram data ...");
    fflush(stdout);
    LoadUnigramFile(unigram_file, &unigram_data, &status);
  }

  int total_count = 0;
  if (status.ok()) {
    printf(" OK, %d entries loaded.\n", static_cast<int>(unigram_data.size()));
    printf("Loading bigram data ...");
    fflush(stdout);
    LoadBigramFile(bigram_file, &bigram_data, &total_count, &status);
  }

  if (status.ok()) {
    printf(" OK, %d entries loaded.\n", static_cast<int>(bigram_data.size()));
    printf("Saveing unigram index and data file ...");
    fflush(stdout);
    BuildAndSaveUnigramData(unigram_data, &double_array, &status);
  }

  int count = 0;
  if (status.ok()) {
    printf(" OK\n");
    printf("Saving Bigram Binary File ...");
    count = SaveBigramBinFile(bigram_data, total_count, double_array, &status);
  }

  if (status.ok()) {
    printf(" OK, %d entries saved.\n", count);
    printf("Success!");
    return 0;
  } else {
    puts(status.what());
    return -1;
  }
}

int MakeIndexFile(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: mc_model dict [INPUT-FILE] [OUTPUT-FILE]\n");
    return 1;
  }

  const char *input_path = argv[argc - 2];
  const char *output_path = argv[argc - 1];

  Darts::DoubleArray double_array;

  FILE *fd = fopen(input_path, "r");
  if (fd == NULL) {
    fprintf(stderr, "error: unable to open input file %s\n", input_path);
    return 1;
  }

  char key_text[1024];
  int value = 0;
  std::vector<std::pair<std::string, int> > key_value;
  while (fscanf(fd, "%s %d", key_text, &value) != EOF) {
    key_value.push_back(std::pair<std::string, int>(key_text, value));
  }
  sort(key_value.begin(), key_value.end());
  printf("read %ld words.\n", key_value.size());

  std::vector<const char *> keys;
  std::vector<Darts::DoubleArray::value_type> values;

  for (std::vector<std::pair<std::string, int> >::iterator
       it = key_value.begin(); it != key_value.end(); ++it) {
    keys.push_back(it->first.c_str());
    values.push_back(it->second);
  }

  if (double_array.build(keys.size(), &keys[0], 0, &values[0]) != 0) {
    fprintf(stderr,
            "error: unable to build double array from file %s\n",
            input_path);
    return 1;
  }

  if (double_array.save(output_path) != 0) {
    fprintf(stderr,
            "error: unable to save double array to file %s\n",
            output_path);
    return 1;
  }

  fclose(fd);
  return 0;
}

int MakeMulticlassPerceptronFile(int argc, char **argv) {
  Status status;

  if (argc != 4)
    status = Status::Info("Usage: mc_model multiperc "
                          "text-model-file binary-model-file");

  printf("Load text formatted model: %s \n", argv[argc - 2]);
  MulticlassPerceptronModel *
  perc = MulticlassPerceptronModel::OpenText(argv[argc - 2], &status);

  if (status.ok()) {
    printf("Save binary formatted model: %s \n", argv[argc - 1]);
    perc->Save(argv[argc - 1], &status);
  }

  delete perc;
  if (status.ok()) {
    return 0;
  } else {
    puts(status.what());
    return -1;
  }
}

void DisplayProgress(int64_t bytes_processed,
                     int64_t file_size,
                     int64_t bytes_per_second) {
  fprintf(stderr,
          "\rprogress %dMB/%dMB -- %2.1f%% %.3fMB/s",
          static_cast<int>(bytes_processed / (1024 * 1024)),
          static_cast<int>(file_size / (1024 * 1024)),
          100.0 * bytes_processed / file_size,
          bytes_per_second / static_cast<double>(1024 * 1024));
}

int TrainNaiveArcEagerDependendyParser(int argc, char **argv) {
  if (argc != 6) {
    fprintf(stderr,
            "Usage: milkcat-tools --depparser-train corpus_file template_file "
            "model_file iteration\n");
    return 1;
  }
  const char *corpus_file = argv[2];
  const char *template_file = argv[3];
  const char *model_prefix = argv[4];
  int max_iteration = atol(argv[5]);

  Status status;
  BeamArceagerDependencyParser::Train(
      corpus_file,
      template_file,
      model_prefix,
      max_iteration,
      &status);

  if (status.ok()) {
    puts("Success!");
    return 0;
  } else {
    puts(status.what());
    return 1;
  }
}

int TestDependendyParser(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr,
            "Usage: milkcat-tools --depparser-test corpus_file template_file "
            "model_file\n");
    return 1;
  }
  const char *corpus_file = argv[2];
  const char *template_file = argv[3];
  const char *model_prefix = argv[4];

  Status status;
  DependencyParser::FeatureTemplate *
  feature = DependencyParser::FeatureTemplate::Open(template_file, &status);

  MulticlassPerceptronModel *model = NULL;
  if (status.ok()) {
    model = MulticlassPerceptronModel::Open(model_prefix, &status);
  } 

  DependencyParser *parser = NULL;
  double LAS, UAS;
  if (status.ok()) {
    parser = new BeamArceagerDependencyParser(model, feature);
    DependencyParser::Test(
        corpus_file,
        parser,
        &LAS,
        &UAS,
        &status);
  }

  if (status.ok()) {
    printf("LAS: %lf\n", LAS);
    printf("UAS: %lf\n", UAS);
  }

  if (!status.ok()) puts(status.what());

  delete feature;
  delete model;
  delete parser;
  return 0;
}

int TestPartOfSpeechTagger(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr,
            "Usage: milkcat-tools --postagger-test corpus_file model_file\n");
    return 1;
  }
  const char *corpus_file = argv[2];
  const char *model_file = argv[3];

  Status status;

  HMMPartOfSpeechTagger *tagger = NULL;
  HMMModel *model = HMMModel::New(model_file, &status);
  if (status.ok()) {
    tagger = HMMPartOfSpeechTagger::New(model, &status);
  }
  double ta = 0.0;
  if (status.ok()) {
    ta = PartOfSpeechTagger::Test(corpus_file, tagger, &status);
  }

  if (!status.ok()) {
    puts(status.what());
  } else {
    printf("TA = %5.2f\n", ta);
  }

  delete tagger;
  delete model;

  return 0;
}

int TrainHmmPartOfSpeechTagger(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr,
            "Usage: milkcat-tools --postagger-train hmm corpus_file"
            " model_file\n");
    return 1;
  }
  const char *corpus_file = argv[3];  
  const char *model_file = argv[4];

  Status status;
  HMMPartOfSpeechTagger::Train(corpus_file, model_file, &status);
  if (!status.ok()) {
    puts(status.what());
    return 1;
  } else {
    return 0;
  }
}

}  // namespace milkcat

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: mc_model [dict|gram|hmm|maxent]\n");
    return 1;
  }

  char *tool = argv[1];

  if (strcmp(tool, "dict") == 0) {
    return milkcat::MakeIndexFile(argc, argv);
  } else if (strcmp(tool, "gram") == 0) {
    return milkcat::MakeGramModel(argc, argv);
  } else if (strcmp(tool, "multiperc") == 0) {
    return milkcat::MakeMulticlassPerceptronFile(argc, argv);
  } else if (strcmp(tool, "--depparser-train") == 0) {
    return milkcat::TrainNaiveArcEagerDependendyParser(argc, argv);
  } else if (strcmp(tool, "--depparser-test") == 0) {
    return milkcat::TestDependendyParser(argc, argv);
  } else if (strcmp(tool, "--postagger-test") == 0) {
    return milkcat::TestPartOfSpeechTagger(argc, argv);
  } else if (strcmp(tool, "--postagger-train") == 0) {
    return milkcat::TrainHmmPartOfSpeechTagger(argc, argv);
  } else {
    fprintf(stderr, "Usage: mc_model [dict|gram|hmm|maxent|vocab|tfidf]\n");
    return 1;
  }

  return 0;
}