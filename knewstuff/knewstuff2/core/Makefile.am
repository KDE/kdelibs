INCLUDES = $(all_includes) -I $(top_srcdir)/src

METASOURCES = AUTO

lib_LTLIBRARIES = libknewstuff2.la

libknewstuff2_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 2:0:1
libknewstuff2_la_LIBADD  = $(LIB_KIO)
libknewstuff2_la_SOURCES = ktranslatable.cpp author.cpp entry.cpp provider.cpp entryhandler.cpp category.cpp providerloader.cpp providerhandler.cpp

#knsdir = $(includedir)/knewstuff2
#kns_HEADERS = entry.h provider.h category.h ktranslatable.h
#noinst_HEADERS = entryhandler.h

