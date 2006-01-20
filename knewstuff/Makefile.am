INCLUDES = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libknewstuff.la

libknewstuff_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libknewstuff_la_LIBADD  = $(LIB_KIO)
libknewstuff_la_SOURCES = engine.cpp entry.cpp downloaddialog.cpp \
  uploaddialog.cpp providerdialog.cpp provider.cpp knewstuff.cpp \
  knewstuffgeneric.cpp knewstuffbutton.cpp knewstuffsecure.cpp security.cpp

bin_PROGRAMS = khotnewstuff

khotnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH)
khotnewstuff_LDADD = libknewstuff.la
khotnewstuff_SOURCES = khotnewstuff.cpp

EXTRA_PROGRAMS = testnewstuff ghns

testnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testnewstuff_LDADD   = libknewstuff.la
testnewstuff_SOURCES = testnewstuff.cpp

ghns_LDFLAGS = $(all_libraries) $(KDE_RPATH)
ghns_LDADD   = libknewstuff.la
ghns_SOURCES = ghns.cpp

kde_conf_DATA = khotnewstuffrc

knsdir = $(includedir)/knewstuff
kns_HEADERS = downloaddialog.h engine.h entry.h knewstuffgeneric.h knewstuff.h providerdialog.h provider.h uploaddialog.h knewstuffbutton.h knewstuffsecure.h
noinst_HEADERS = testnewstuff.h ghns.h security.h

KDE_ICON = AUTO

include ../admin/Doxyfile.am

