noinst_LIBRARIES = libdasherlm.a

if JAPANESE
japanese_source = JapaneseLanguageModel.cpp JapaneseLanguageModel.h KanjiConversion.h KanjiConversionCanna.cpp  KanjiConversionCanna.h
japanese_extra_dist = 
else
japanese_source =
japanese_extra_dist = JapaneseLanguageModel.cpp JapaneseLanguageModel.h KanjiConversion.h KanjiConversionCanna.cpp  KanjiConversionCanna.h
endif

libdasherlm_a_SOURCES = DictLanguageModel.cpp \
                        DictLanguageModel.h \
                        $(japanese_source) \
                        LanguageModel.cpp \
                        LanguageModel.h \
                        LanguageModelParams.h \
                        MixtureLanguageModel.h \
                        PPMLanguageModel.cpp \
                        PPMLanguageModel.h \
                        SymbolAlphabet.h \
                        WordLanguageModel.cpp \
                        WordLanguageModel.h

EXTRA_DIST = $(japanese_extra_dist)
