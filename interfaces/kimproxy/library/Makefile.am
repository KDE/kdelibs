# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = kimproxyiface.h imbutton.h imlabel.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

# maybe this will make it find DCOPClient?
AM_CPPFLAGS = $(all_includes)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kimproxy.pot

# maybe in kdepim?
kimproxyincludedir = $(includedir)
kimproxyinclude_HEADERS = kimproxy.h kimproxyiface.h

lib_LTLIBRARIES =  libkimproxy.la
libkimproxy_la_LDFLAGS = $(all_libraries)
libkimproxy_la_LIBADD = $(LIB_KDECORE)
libkimproxy_la_SOURCES = kimproxy.cpp kimiface.stub kimproxyiface.skel imbutton.cpp imlabel.cpp imlabelbase.ui

kimiface_DIR = $(top_srcdir)/kimproxy/interfaces
