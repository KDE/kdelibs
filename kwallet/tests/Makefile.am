AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_ASCII_CAST -I$(srcdir)/../client $(all_includes)

METASOURCES = AUTO
check_PROGRAMS = kwalletasync kwalletsync kwalletboth

kwalletasync_SOURCES = kwalletasync.cpp kwallettest.cpp
kwalletasync_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kwalletasync_LDADD = $(LIB_QT) $(LIB_KDECORE) -L../client -lkwalletclient

kwalletsync_SOURCES = kwalletsync.cpp
kwalletsync_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kwalletsync_LDADD = $(LIB_QT) $(LIB_KDECORE) -L../client -lkwalletclient

kwalletboth_SOURCES = kwalletboth.cpp kwallettest.cpp
kwalletboth_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kwalletboth_LDADD = $(LIB_QT) $(LIB_KDECORE) -L../client -lkwalletclient
