INCLUDES=$(all_includes)

lib_LTLIBRARIES = libkntlm.la
METASOURCES     = AUTO

kntlmincludedir = $(includedir)/kio
kntlminclude_HEADERS = kntlm.h

libkntlm_la_SOURCES = kntlm.cpp des.cpp
libkntlm_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -no-undefined
libkntlm_la_LIBADD = $(LIB_KDECORE)

