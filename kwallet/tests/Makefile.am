AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_ASCII_CAST -I$(srcdir)/../client $(all_includes)

METASOURCES = AUTO
check_PROGRAMS = kwalletasync kwalletsync kwalletboth

kwalletasync_SOURCES = kwalletasync.cpp kwallettest.cpp
kwalletasync_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwalletasync_LDADD = ../client/libkwalletclient.la $(LIB_KDECORE)

kwalletsync_SOURCES = kwalletsync.cpp
kwalletsync_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwalletsync_LDADD = ../client/libkwalletclient.la $(LIB_KDECORE)

kwalletboth_SOURCES = kwalletboth.cpp kwallettest.cpp
kwalletboth_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwalletboth_LDADD = ../client/libkwalletclient.la $(LIB_KDECORE)
