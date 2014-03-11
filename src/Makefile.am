AUTOMAKE_OPTIONS = gnu subdir-objects
AM_CXXFLAGS = -I$(top_srcdir)/src -DMODEL_PATH=\"$(pkgdatadir)/\" -std=c++11 -fno-rtti \
              -fno-exceptions -Wall -Werror -pthread

ACLOCAL_AMFLAGS = -I m4

include_HEADERS = milkcat/milkcat.h

lib_LTLIBRARIES = libmilkcat.la
libmilkcat_la_SOURCES = common/get_vocabulary.cc \
                        common/get_vocabulary.hmilkcat/beam.h \
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
                        milkcat/darts.h \
                        milkcat/feature_extractor.h \
                        milkcat/hmm_model.cc \
                        milkcat/hmm_model.h \
                        milkcat/hmm_part_of_speech_tagger.cc \
                        milkcat/hmm_part_of_speech_tagger.h \
                        milkcat/instance_data.cc \
                        milkcat/instance_data.h \
                        milkcat/libmilkcat.cc \
                        milkcat/libmilkcat.h \
                        milkcat/milkcat.h \
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
                        milkcat/trie_tree.h \
                        milkcat/trie_tree.cc \
                        milkcat/segmenter.h \
                        milkcat/static_array.h \
                        milkcat/static_hashtable.h \
                        neko/bigram_anal.cc \
                        neko/bigram_anal.h \
                        neko/candidate.cc \
                        neko/candidate.h \
                        neko/crf_vocab.cc \
                        neko/crf_vocab.h \
                        neko/final_rank.cc \
                        neko/final_rank.h \
                        neko/maxent_classifier.cc \
                        neko/maxent_classifier.h \
                        neko/mutual_information.cc \
                        neko/mutual_information.h \
                        neko/utf8.h \
                        utils/readable_file.cc \
                        utils/readable_file.h \
                        utils/status.h \
                        utils/strlcpy.cc \
                        utils/utils.cc \
                        utils/utils.h \
                        utils/writable_file.cc \
                        utils/writable_file.h
