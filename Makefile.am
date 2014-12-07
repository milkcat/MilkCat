ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = gnu subdir-objects foreign
AM_CXXFLAGS = -I$(top_srcdir)/src -DMODEL_PATH=\"$(pkgdatadir)/\" -fno-rtti \
              -fno-exceptions -fPIC

dist_pkgdata_DATA = data/bigram.bin \
                    data/ctb_pos.crf \
                    data/ctb_pos.hmm \
                    data/ctb_pos.hmm.x.idx \
                    data/ctb_seg.crf \
                    data/oov_property.idx \
                    data/unigram.bin \
                    data/unigram.idx \
                    data/stopword.idx \
                    data/tfidf.bin \
                    data/depparse.templ

include_HEADERS = src/include/milkcat.h

lib_LIBRARIES = libmilkcat.a
libmilkcat_a_SOURCES = src/libmilkcat.cc \
                       src/libmilkcat_capi.cc \
                       src/libmilkcat.h \
                       src/common/cedar.h \
                       src/common/darts.h \
                       src/common/instance_data.cc \
                       src/common/instance_data.h \
                       src/common/milkcat_config.h \
                       src/common/model_impl.cc \
                       src/common/model_impl.h \
                       src/common/reimu_trie.cc \
                       src/common/static_array.h \
                       src/common/static_hashtable.h \
                       src/common/trie_tree.cc \
                       src/common/trie_tree.h \
                       src/include/milkcat.h \
                       src/ml/averaged_multiclass_perceptron.cc \
                       src/ml/averaged_multiclass_perceptron.h \
                       src/ml/beam.h \
                       src/ml/crf_model.cc \
                       src/ml/crf_model.h \
                       src/ml/crf_tagger.cc \
                       src/ml/crf_tagger.h \
                       src/ml/feature_set.h \
                       src/ml/feature_set.cc \
                       src/ml/hmm_model.cc \
                       src/ml/hmm_model.h \
                       src/ml/multiclass_perceptron.cc \
                       src/ml/multiclass_perceptron.h \
                       src/ml/multiclass_perceptron_model.cc \
                       src/ml/multiclass_perceptron_model.h \
                       src/parser/beam_arceager_dependency_parser.h \
                       src/parser/beam_arceager_dependency_parser.cc \
                       src/parser/dependency_parser.cc \
                       src/parser/dependency_parser.h \
                       src/parser/feature_template.cc \
                       src/parser/feature_template.h \
                       src/parser/naive_arceager_dependency_parser.h \
                       src/parser/naive_arceager_dependency_parser.cc \
                       src/parser/node.h \
                       src/parser/orcale.cc \
                       src/parser/orcale.h \
                       src/parser/state.cc \
                       src/parser/state.h \
                       src/parser/tree_instance.cc \
                       src/parser/tree_instance.h \
                       src/segmenter/bigram_segmenter.cc \
                       src/segmenter/bigram_segmenter.h \
                       src/segmenter/crf_segmenter.cc \
                       src/segmenter/crf_segmenter.h \
                       src/segmenter/mixed_segmenter.cc \
                       src/segmenter/mixed_segmenter.h \
                       src/segmenter/out_of_vocabulary_word_recognition.cc \
                       src/segmenter/out_of_vocabulary_word_recognition.h \
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
                       src/utils/mutex.h \
                       src/utils/mutex_posix.cc \
                       src/utils/pool.h \
                       src/utils/readable_file.cc \
                       src/utils/readable_file.h \
                       src/utils/status.h \
                       src/utils/string_builder.h \
                       src/utils/strlcpy.cc \
                       src/utils/utils.cc \
                       src/utils/utils.h \
                       src/utils/utils_posix.cc \
                       src/utils/writable_file.cc \
                       src/utils/writable_file.h 

bin_PROGRAMS = milkcat mctools

milkcat_SOURCES = src/main.cc
milkcat_LDADD = libmilkcat.a

mctools_SOURCES = src/mctools.cc
mctools_LDADD = libmilkcat.a

TESTS = milkcat_capi_test parser_orcale_test reimu_trie_test
check_PROGRAMS = milkcat_capi_test parser_orcale_test reimu_trie_test

milkcat_capi_test_SOURCES = test/milkcat_capi_test.c
milkcat_capi_test_CFLAGS = -DMODEL_DIR=\"$(top_srcdir)/data/\" -lstdc++
milkcat_capi_test_LDADD = libmilkcat.a

parser_orcale_test_SOURCES = test/parser_orcale_test.cc
parser_orcale_test_LDADD = libmilkcat.a

reimu_trie_test_SOURCES = test/reimu_trie_test.cc
reimu_trie_test_LDADD = libmilkcat.a