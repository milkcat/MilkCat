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
                        keyphrase/document.cc \
                        keyphrase/document.h \
                        keyphrase/libkeyphrase.cc \
                        keyphrase/phrase.h \
                        keyphrase/phrase_ranker.h \
                        keyphrase/phrase_ranker.cc \
                        keyphrase/phrase_extractor.h \
                        keyphrase/phrase_extractor.cc \
                        keyphrase/string_table.h \
                        keyphrase/string_value.h \
                        milkcat/beam.h \
                        milkcat/bigram_segmenter.cc \
                        milkcat/bigram_segmenter.h \
                        milkcat/crf_model.cc \
                        milkcat/crf_model.h \
                        milkcat/crf_part_of_speech_tagger.cc \
                        milkcat/crf_part_of_speech_tagger.h \
                        milkcat/crf_segmenter.cc \
                        milkcat/crf_segmenter.h \
                        milkcat/crf_tagger.cc \
                        milkcat/crf_tagger.h \
                        milkcat/dependency_instance.h \
                        milkcat/dependency_instance.cc \
                        milkcat/dependency_parser.h \
                        milkcat/dependency_parser.cc \
                        milkcat/dependency_parser_features.cc \
                        milkcat/feature_extractor.h \
                        milkcat/hmm_model.cc \
                        milkcat/hmm_model.h \
                        milkcat/hmm_part_of_speech_tagger.cc \
                        milkcat/hmm_part_of_speech_tagger.h \
                        milkcat/instance_data.cc \
                        milkcat/instance_data.h \
                        milkcat/libmilkcat.cc \
                        milkcat/libmilkcat.h \
                        milkcat/libmilkcat-c.cc \
                        milkcat/milkcat_config.h \
                        milkcat/mixed_segmenter.cc \
                        milkcat/mixed_segmenter.h \
                        milkcat/out_of_vocabulary_word_recognition.cc \
                        milkcat/out_of_vocabulary_word_recognition.h \
                        milkcat/part_of_speech_tag_instance.cc \
                        milkcat/part_of_speech_tag_instance.h \
                        milkcat/part_of_speech_tagger.h \
                        milkcat/term_instance.h \
                        milkcat/term_instance.cc \
                        milkcat/token_instance.h \
                        milkcat/token_instance.cc \
                        milkcat/token_lex.cc \
                        milkcat/token_lex.h \
                        milkcat/tokenizer.cc \
                        milkcat/tokenizer.h \
                        milkcat/segmenter.h \
                        milkcat/static_array.h \
                        milkcat/static_hashtable.h \
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
                        nekoneko/candidate.cc \
                        nekoneko/extract_adjent.cc \
                        nekoneko/final_rank.cc \
                        nekoneko/get_crf_vocabulary.cc \
                        nekoneko/libnekoneko.cc \
                        nekoneko/libnekoneko.h \
                        nekoneko/mutual_information.cc \
                        nekoneko/utf8.h

TESTS = dependency_test sparsehash_test
check_PROGRAMS = dependency_test sparsehash_test

dependency_test_SOURCES = test/dependency_test.cc
dependency_test_CXXFLAGS = -DMODEL_DIR=\"$(top_srcdir)/data/\" \
                           -DPRIVATE_DIR=\"$(top_srcdir)/private/\"
dependency_test_LDFLAGS = -all-static
dependency_test_LDADD = libmilkcat.la

sparsehash_test_SOURCES = test/sparsehash_test.cc
