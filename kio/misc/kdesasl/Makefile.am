INCLUDES=$(all_includes)

lib_LTLIBRARIES = libkdesasl.la
METASOURCES     = AUTO

kdesaslincludedir = $(includedir)/kio
kdesaslinclude_HEADERS = kdesasl.h

libkdesasl_la_SOURCES = kdesasl.cpp
libkdesasl_la_LDFLAGS = $(all_libraries) -version-info 3:0:2 -no-undefined
libkdesasl_la_LIBADD = $(LIB_KDECORE)

