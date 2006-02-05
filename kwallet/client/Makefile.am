
INCLUDES= -I$(srcdir) $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

lib_LTLIBRARIES = libkwalletclient.la

libkwalletclient_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:1 -no-undefined
libkwalletclient_la_LIBADD = $(LIB_KDECORE)
libkwalletclient_la_SOURCES =   kwallet.skel \
				kwallet.cc

libkwalletclient_la_METASOURCES = AUTO

include_HEADERS = kwallet.h kwallettypes.h

