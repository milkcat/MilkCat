AUTOMAKE_OPTIONS = gnu subdir-objects
AM_CFLAGS = -fPIC
AM_CXXFLAGS = -I$(top_srcdir)/src -DMODEL_PATH=\"$(pkgdatadir)/\" -fno-rtti \
              -fno-exceptions

ACLOCAL_AMFLAGS = -I m4

include_HEADERS = include/milkcat.h

lib_LTLIBRARIES = libmilkcat.la
libmilkcat_la_SOURCES = libmilkcat.cc \
                        libmilkcat.h \
                        common/cedar.h \
                        common/darts.h \
                        common/instance_data.cc \
                        common/instance_data.h \
                        common/milkcat_config.h \
                        common/model_impl.cc \
                        common/model_impl.h \
                        common/reimu_trie.h \
                        common/reimu_trie.cc \
                        common/static_array.h \
                        common/static_hashtable.h \
                        common/trie_tree.cc \
                        common/trie_tree.h \
                        include/milkcat.h \
                        ml/beam.h \
                        ml/crf_model.cc \
                        ml/crf_model.h \
                        ml/crf_tagger.cc \
                        ml/crf_tagger.h \
                        ml/feature_set.h \
                        ml/feature_set.cc \
                        ml/hmm_model.cc \
                        ml/hmm_model.h \
                        ml/multiclass_perceptron.cc \
                        ml/multiclass_perceptron.h \
                        ml/multiclass_perceptron_model.cc \
                        ml/multiclass_perceptron_model.h \
                        ml/sequence_feature_extractor.h \
                        parser/dependency_instance.h \
                        parser/dependency_instance.cc \
                        parser/dependency_parser.cc \
                        parser/dependency_parser.h \
                        parser/feature_template.cc \
                        parser/feature_template.h \
                        parser/naive_arceager_dependency_parser.h \
                        parser/naive_arceager_dependency_parser.cc \
                        parser/node.h \
                        parser/orcale.cc \
                        parser/orcale.h \
                        parser/state.cc \
                        parser/state.h \
                        segmenter/bigram_segmenter.cc \
                        segmenter/bigram_segmenter.h \
                        segmenter/crf_segmenter.cc \
                        segmenter/crf_segmenter.h \
                        segmenter/mixed_segmenter.cc \
                        segmenter/mixed_segmenter.h \
                        segmenter/out_of_vocabulary_word_recognition.cc \
                        segmenter/out_of_vocabulary_word_recognition.h \
                        segmenter/segmenter.h \
                        segmenter/term_instance.h \
                        segmenter/term_instance.cc \
                        tagger/crf_part_of_speech_tagger.cc \
                        tagger/crf_part_of_speech_tagger.h \
                        tagger/hmm_part_of_speech_tagger.cc \
                        tagger/hmm_part_of_speech_tagger.h \
                        tagger/part_of_speech_tag_instance.cc \
                        tagger/part_of_speech_tag_instance.h \
                        tagger/part_of_speech_tagger.h \
                        tokenizer/token_instance.h \
                        tokenizer/token_instance.cc \
                        tokenizer/token_lex.cc \
                        tokenizer/token_lex.h \
                        tokenizer/tokenizer.cc \
                        tokenizer/tokenizer.h \
                        utils/log.cc \
                        utils/log.h \
                        utils/mutex.h \
                        utils/mutex_posix.cc \
                        utils/pool.h \
                        utils/readable_file.cc \
                        utils/readable_file.h \
                        utils/status.h \
                        utils/string_builder.h \
                        utils/strlcpy.cc \
                        utils/utils.cc \
                        utils/utils.h \
                        utils/utils_posix.cc \
                        utils/writable_file.cc \
                        utils/writable_file.h 

TESTS = dependency_test reimu_trie_test orcale_test
check_PROGRAMS = dependency_test reimu_trie_test orcale_test 

reimu_trie_test_SOURCES = tests/reimu_trie_test.cc
reimu_trie_test_LDADD = libmilkcat.la

orcale_test_SOURCES = tests/orcale_test.cc
orcale_test_LDADD = libmilkcat.la


