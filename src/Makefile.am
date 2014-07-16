AUTOMAKE_OPTIONS = gnu subdir-objects
AM_CFLAGS = -fPIC
AM_CXXFLAGS = -I$(top_srcdir)/src -DMODEL_PATH=\"$(pkgdatadir)/\" -fno-rtti \
              -fno-exceptions

ACLOCAL_AMFLAGS = -I m4

include_HEADERS = include/milkcat.h include/milkcat-c.h

lib_LTLIBRARIES = libmilkcat.la
libmilkcat_la_LIBADD = -lpthread
libmilkcat_la_LDFLAGS = -version-info 0:3:0
libmilkcat_la_SOURCES = common/cedar.h \
                        common/darts.h \
                        common/get_vocabulary.h \
                        common/get_vocabulary.cc \
                        common/maxent_classifier.cc \
                        common/maxent_classifier.h \
                        common/milkcat_config.h \
                        common/model_factory.cc \
                        common/model_factory.h \
                        common/trie_tree.cc \
                        common/trie_tree.h \
                        include/milkcat.h \
                        include/milkcat-c.h \
                        parser/beam.h \
                        parser/bigram_segmenter.cc \
                        parser/bigram_segmenter.h \
                        parser/crf_model.cc \
                        parser/crf_model.h \
                        parser/crf_part_of_speech_tagger.cc \
                        parser/crf_part_of_speech_tagger.h \
                        parser/crf_segmenter.cc \
                        parser/crf_segmenter.h \
                        parser/crf_tagger.cc \
                        parser/crf_tagger.h \
                        parser/dependency_instance.h \
                        parser/dependency_instance.cc \
                        parser/dependency_parser.h \
                        parser/dependency_parser.cc \
                        parser/dependency_parser_features.cc \
                        parser/feature_extractor.h \
                        parser/hmm_model.cc \
                        parser/hmm_model.h \
                        parser/hmm_part_of_speech_tagger.cc \
                        parser/hmm_part_of_speech_tagger.h \
                        parser/instance_data.cc \
                        parser/instance_data.h \
                        parser/libmilkcat.cc \
                        parser/libmilkcat.h \
                        parser/libmilkcat-c.cc \
                        parser/milkcat_config.h \
                        parser/mixed_segmenter.cc \
                        parser/mixed_segmenter.h \
                        parser/out_of_vocabulary_word_recognition.cc \
                        parser/out_of_vocabulary_word_recognition.h \
                        parser/part_of_speech_tag_instance.cc \
                        parser/part_of_speech_tag_instance.h \
                        parser/part_of_speech_tagger.h \
                        parser/term_instance.h \
                        parser/term_instance.cc \
                        parser/token_instance.h \
                        parser/token_instance.cc \
                        parser/token_lex.cc \
                        parser/token_lex.h \
                        parser/tokenizer.cc \
                        parser/tokenizer.h \
                        parser/segmenter.h \
                        parser/static_array.h \
                        parser/static_hashtable.h \
                        phrase/document.cc \
                        phrase/document.h \
                        phrase/phrase.cc \
                        phrase/phrase.h \
                        phrase/phrase_extractor.h \
                        phrase/phrase_extractor.cc \
                        phrase/string_table.h \
                        phrase/string_value.h \
                        utils/log.h \
                        utils/log.cc \
                        utils/readable_file.cc \
                        utils/readable_file.h \
                        utils/status.h \
                        utils/string_builder.h \
                        utils/strlcpy.cc \
                        utils/utils.cc \
                        utils/utils.h \
                        utils/utils_posix.cc \
                        utils/writable_file.cc \
                        utils/writable_file.h \
                        utils/mutex.h \
                        utils/mutex_posix.cc \
                        utils/thread.h \
                        utils/thread_posix.cc \
                        newword/candidate.cc \
                        newword/adjent_entropy.cc \
                        newword/rank.cc \
                        newword/vocabulary.cc \
                        newword/newword.cc \
                        newword/newword.h \
                        newword/mutual_information.cc \
                        newword/utf8.h

TESTS = dependency_test 
check_PROGRAMS = dependency_test 

dependency_test_SOURCES = tests/dependency_test.cc
dependency_test_CXXFLAGS = -DMODEL_DIR=\"$(top_srcdir)/data/\" \
                           -DPRIVATE_DIR=\"$(top_srcdir)/private/\"
dependency_test_LDFLAGS = -all-static
dependency_test_LDADD = libmilkcat.la

