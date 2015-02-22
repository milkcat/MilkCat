ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = gnu subdir-objects foreign
AM_CXXFLAGS = -I$(top_srcdir)/src -DMODEL_PATH=\"$(pkgdatadir)/\" -fno-rtti \
              -fno-exceptions -fPIC

dist_pkgdata_DATA = data/bigram.bin \
                    data/ctb_pos.crf.cost.bi \
                    data/ctb_pos.crf.cost.uni \
                    data/ctb_pos.crf.meta \
                    data/ctb_pos.crf.x.idx \
                    data/ctb_pos.hmm \
                    data/ctb_pos.hmm.x.idx \
                    data/ctb_seg.crf.cost.bi \
                    data/ctb_seg.crf.cost.uni \
                    data/ctb_seg.crf.meta \
                    data/ctb_seg.crf.x.idx \
                    data/oov_property.idx \
                    data/unigram.bin \
                    data/unigram.idx \
                    data/depparse.tmpl \
                    data/ctb_dep.b1.meta \
                    data/ctb_dep.b1.x.idx \
                    data/ctb_dep.b1.cost.data \
                    data/ctb_dep.b8.meta \
                    data/ctb_dep.b8.x.idx \
                    data/ctb_dep.b8.cost.data

include_HEADERS = src/include/milkcat.h

lib_LIBRARIES = libmilkcat.a
libmilkcat_a_SOURCES = src/libmilkcat.cc \
                       src/libmilkcat_capi.cc \
                       src/libmilkcat.h \
                       src/common/instance_data.cc \
                       src/common/instance_data.h \
                       src/common/milkcat_config.h \
                       src/common/model_impl.cc \
                       src/common/model_impl.h \
                       src/common/reimu_trie.cc \
                       src/common/reimu_trie.h \
                       src/common/static_array.h \
                       src/common/static_hashtable.h \
                       src/include/milkcat.h \
                       src/ml/beam.h \
                       src/ml/crf_model.cc \
                       src/ml/crf_model.h \
                       src/ml/crf_tagger.cc \
                       src/ml/crf_tagger.h \
                       src/ml/feature_set.h \
                       src/ml/feature_set.cc \
                       src/ml/hmm_model.cc \
                       src/ml/hmm_model.h \
                       src/ml/packed_score.h \
                       src/ml/perceptron.cc \
                       src/ml/perceptron.h \
                       src/ml/perceptron_model.cc \
                       src/ml/perceptron_model.h \
                       src/ml/sequence_feature_set.h \
                       src/parser/beam_yamada_parser.h \
                       src/parser/beam_yamada_parser.cc \
                       src/parser/dependency_parser.cc \
                       src/parser/dependency_parser.h \
                       src/parser/feature_template.cc \
                       src/parser/feature_template.h \
                       src/parser/feature_template-inl.h \
                       src/parser/node.h \
                       src/parser/orcale.cc \
                       src/parser/orcale.h \
                       src/parser/state.cc \
                       src/parser/state.h \
                       src/parser/tree_instance.cc \
                       src/parser/tree_instance.h \
                       src/parser/yamada_parser.h \
                       src/parser/yamada_parser.cc \
                       src/segmenter/bigram_segmenter.cc \
                       src/segmenter/bigram_segmenter.h \
                       src/segmenter/crf_segmenter.cc \
                       src/segmenter/crf_segmenter.h \
                       src/segmenter/mixed_segmenter.cc \
                       src/segmenter/mixed_segmenter.h \
                       src/segmenter/out_of_vocabulary_word_recognizer.cc \
                       src/segmenter/out_of_vocabulary_word_recognizer.h \
                       src/segmenter/segmenter.h \
                       src/segmenter/term_instance.h \
                       src/segmenter/term_instance.cc \
                       src/tagger/crf_part_of_speech_tagger.cc \
                       src/tagger/crf_part_of_speech_tagger.h \
                       src/tagger/hmm_part_of_speech_tagger.cc \
                       src/tagger/hmm_part_of_speech_tagger.h \
                       src/tagger/part_of_speech_tag_instance.cc \
                       src/tagger/part_of_speech_tag_instance.h \
                       src/tagger/part_of_speech_tagger.cc \
                       src/tagger/part_of_speech_tagger.h \
                       src/tokenizer/token_instance.h \
                       src/tokenizer/token_instance.cc \
                       src/tokenizer/token_lex.cc \
                       src/tokenizer/token_lex.h \
                       src/tokenizer/tokenizer.cc \
                       src/tokenizer/tokenizer.h \
                       src/util/encoding.h \
                       src/util/encoding_posix.cc \
                       src/util/mutex.h \
                       src/util/mutex_posix.cc \
                       src/util/pool.h \
                       src/util/readable_file.cc \
                       src/util/readable_file.h \
                       src/util/status.h \
                       src/util/string_builder.h \
                       src/util/strlcpy.cc \
                       src/util/strtok_r.cc \
                       src/util/util_posix.cc \
                       src/util/util.cc \
                       src/util/util.h \
                       src/util/writable_file.cc \
                       src/util/writable_file.h 

bin_PROGRAMS = milkcat milkcat-tools

milkcat_SOURCES = src/main.cc
milkcat_LDADD = libmilkcat.a -liconv -lm

milkcat_tools_SOURCES = src/milkcat_tools.cc
milkcat_tools_LDADD = libmilkcat.a -liconv -lm

TESTS = milkcat_api_test milkcat_capi_test parser_orcale_test reimu_trie_test
check_PROGRAMS = milkcat_api_test \
                 milkcat_capi_test \
                 parser_orcale_test \
                 reimu_trie_test

milkcat_capi_test_SOURCES = test/milkcat_capi_test.c
milkcat_capi_test_CFLAGS = -DMODEL_DIR=\"$(top_srcdir)/data/\" -lstdc++ -I../src
milkcat_capi_test_LDADD = libmilkcat.a

milkcat_api_test_SOURCES = test/milkcat_api_test.cc
milkcat_api_test_CXXFLAGS = -DMODEL_DIR=\"$(top_srcdir)/data/\" -I../src
milkcat_api_test_LDADD = libmilkcat.a

parser_orcale_test_SOURCES = test/parser_orcale_test.cc
parser_orcale_test_LDADD = libmilkcat.a

reimu_trie_test_SOURCES = test/reimu_trie_test.cc
reimu_trie_test_LDADD = libmilkcat.a