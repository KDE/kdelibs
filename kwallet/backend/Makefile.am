
INCLUDES= -I$(srcdir) $(all_includes) -I../client

lib_LTLIBRARIES = libkwalletbackend.la

libkwalletbackend_la_LDFLAGS = $(all_libraries) -version-info 1:0 -no-undefined
libkwalletbackend_la_LIBADD = $(LIB_QT) -L../client -lkdecore -lkwalletclient
libkwalletbackend_la_SOURCES = blockcipher.cc		\
			       blowfish.cc		\
			       cbc.cc			\
			       hashbase.cc		\
			       sha1.cc			\
			       kwalletbackend.cc


libkwalletbackend_la_METASOURCES = AUTO

noinst_HEADERS = blowfishtables.h	\
		 cbc.h			\
		 hashbase.h		\
		 sha1.h			\
		 blockcipher.h		\
		 blowfish.h



METASOURCES = AUTO




