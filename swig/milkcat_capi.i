%module milkcat_capi

%{

#include <stdbool.h>

%}

typedef struct milkcat_parser_t milkcat_parser_t;

typedef struct milkcat_parseriter_internal_t milkcat_parseriter_internal_t;
typedef struct milkcat_parseriterator_t {
  %immutable;
  const char *word;
  const char *part_of_speech_tag;
  int head;
  const char *dependency_label;
  bool is_begin_of_sentence;
  milkcat_parseriter_internal_t *it;
} milkcat_parseriterator_t;

#define MC_SEGMENTER_BIGRAM 0
#define MC_SEGMENTER_CRF 1
#define MC_SEGMENTER_MIXED 2

#define MC_POSTAGGER_MIXED 0
#define MC_POSTAGGER_CRF 1
#define MC_POSTAGGER_HMM 2
#define MC_POSTAGGER_NONE 3

#define MC_DEPPARSER_YAMADA 0
#define MC_DEPPARSER_BEAMYAMADA 1
#define MC_DEPPARSER_NONE 2

typedef struct milkcat_parseroptions_t {
  int word_segmenter;
  int part_of_speech_tagger;
  int dependency_parser;
  char *user_dictionary_path;
  char *model_path;
  bool use_gbk;
} milkcat_parseroptions_t;

void milkcat_parseroptions_init(milkcat_parseroptions_t *parseropt);
milkcat_parser_t *milkcat_parser_new(milkcat_parseroptions_t *parseropt);
void milkcat_parser_destroy(milkcat_parser_t *model);
void milkcat_parser_predict(
    milkcat_parser_t *parser,
    milkcat_parseriterator_t *parseriter,
    const char *text);

milkcat_parseriterator_t *milkcat_parseriterator_new(void);
void milkcat_parseriterator_destroy(milkcat_parseriterator_t *parseriter);
bool milkcat_parseriterator_next(milkcat_parseriterator_t *parseriter);

const char *milkcat_last_error();
