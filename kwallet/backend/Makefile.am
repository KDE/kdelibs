
INCLUDES= -I$(srcdir) -I$(srcdir)/../client $(all_includes)

lib_LTLIBRARIES = libkwalletbackend.la

libkwalletbackend_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0 -no-undefined
libkwalletbackend_la_LIBADD = $(LIB_QT) ../../kdecore/libkdecore.la ../client/libkwalletclient.la
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
		 kwalletbackend.h	\
		 blowfish.h

