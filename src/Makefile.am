AUTOMAKE_OPTIONS = gnu subdir-objects
AM_CFLAGS = -fPIC
AM_CXXFLAGS = -I$(top_srcdir)/src -DMODEL_PATH=\"$(pkgdatadir)/\" -fno-rtti \
              -fno-exceptions

ACLOCAL_AMFLAGS = -I m4

include_HEADERS = include/milkcat.h

lib_LTLIBRARIES = libmilkcat.la
libmilkcat_la_LIBADD = -lpthread
libmilkcat_la_LDFLAGS = -version-info 0:3:0
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
                        common/word_frequency_count.h \
                        common/word_frequency_count.cc \
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
                        ml/maxent_classifier.cc \
                        ml/maxent_classifier.h \
                        ml/multiclass_perceptron.cc \
                        ml/multiclass_perceptron.h \
                        ml/multiclass_perceptron_model.cc \
                        ml/multiclass_perceptron_model.h \
                        ml/sequence_feature_extractor.h \
                        newword/adjent_entropy.cc \
                        newword/candidate.cc \
                        newword/mutual_information.cc \
                        newword/newword.cc \
                        newword/newword.h \
                        newword/rank.cc \
                        newword/utf8.h \
                        newword/vocabulary.cc \
                        parser/dependency_feature.h \
                        parser/dependency_feature.cc \
                        parser/dependency_instance.h \
                        parser/dependency_instance.cc \
                        parser/dependency_node.h \
                        parser/dependency_parser.h \
                        parser/dependency_state.cc \
                        parser/dependency_state.h \
                        parser/naive_arceager_dependency_parser.h \
                        parser/naive_arceager_dependency_parser.cc \
                        phrase/document.cc \
                        phrase/document.h \
                        phrase/phrase.cc \
                        phrase/phrase_extractor.h \
                        phrase/phrase_extractor.cc \
                        phrase/string_table.h \
                        phrase/string_value.h \
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
                        utils/thread.h \
                        utils/thread_posix.cc \
                        utils/utils.cc \
                        utils/utils.h \
                        utils/utils_posix.cc \
                        utils/writable_file.cc \
                        utils/writable_file.h 

TESTS = dependency_test reimu_trie_test
check_PROGRAMS = dependency_test reimu_trie_test

dependency_test_SOURCES = tests/dependency_test.cc
dependency_test_CXXFLAGS = -DMODEL_DIR=\"$(top_srcdir)/data/\" \
                           -DPRIVATE_DIR=\"$(top_srcdir)/private/\"
dependency_test_LDFLAGS = -all-static
dependency_test_LDADD = libmilkcat.la

reimu_trie_test_SOURCES = tests/reimu_trie_test.cc
reimu_trie_test_LDADD = libmilkcat.la
