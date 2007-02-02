AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_ASCII_CAST $(all_includes)

METASOURCES = AUTO
check_PROGRAMS = kwalletasync kwalletsync kwalletboth

kwalletasync_SOURCES = kwalletasync.cpp kwallettest.cpp
kwalletasync_LDFLAGS = -no-undefined $(KDE_RPATH) $(all_libraries)
kwalletasync_LDADD = $(top_builddir)/kwallet/client/libkwalletclient.la \
	$(LIB_KDECORE) $(LIB_QT)

kwalletsync_SOURCES = kwalletsync.cpp
kwalletsync_LDFLAGS = -no-undefined $(KDE_RPATH) $(all_libraries)
kwalletsync_LDADD = $(top_builddir)/kwallet/client/libkwalletclient.la \
	$(LIB_KDECORE) $(LIB_QT)

kwalletboth_SOURCES = kwalletboth.cpp kwallettest.cpp
kwalletboth_LDFLAGS = -no-undefined $(KDE_RPATH) $(all_libraries)
kwalletboth_LDADD = $(top_builddir)/kwallet/client/libkwalletclient.la \
	$(LIB_KDECORE) $(LIB_QT)
INCLUDES = -I$(top_srcdir)/kwallet/client
