
INCLUDES= -I$(srcdir) $(all_includes) -I$(srcdir)/../client

lib_LTLIBRARIES = libkwalletbackend.la

libkwalletbackend_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0 -no-undefined
libkwalletbackend_la_LIBADD = $(LIB_QT) ../../kdecore/libkdecore.la
libkwalletbackend_la_SOURCES = blockcipher.cc		\
			       blowfish.cc		\
			       cbc.cc			\
			       hashbase.cc		\
			       sha1.cc			\
			       kwalletentry.cc		\
			       kwalletbackend.cc


libkwalletbackend_la_METASOURCES = AUTO

noinst_HEADERS = blowfishtables.h	\
		 cbc.h			\
		 hashbase.h		\
		 sha1.h			\
		 blockcipher.h		\
		 kwalletentry.h		\
		 kwalletbackend.h	\
		 blowfish.h

