AM_CPPFLAGS = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES =  libktts.la
libktts_la_LDFLAGS = -avoid-version -module $(all_libraries)
libktts_la_SOURCES =  ktts.cpp kspeech.skel kspeech.stub kspeechsink.skel

noinst_HEADERS = ktts.h
include_HEADERS = kspeech.h kspeech_stub.h kspeechsink.h

