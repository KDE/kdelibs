
INCLUDES= -I$(srcdir) $(all_includes)

lib_LTLIBRARIES = libkwalletclient.la

libkwalletclient_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0 -no-undefined
libkwalletclient_la_LIBADD = $(LIB_QT) ../../dcop/libDCOP.la
libkwalletclient_la_SOURCES =	kwalletentry.cc \
				kwalletnetworkpassword.cc \
				kwallet.cc

libkwalletclient_la_METASOURCES = AUTO

