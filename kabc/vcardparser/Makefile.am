INCLUDES = $(all_includes) -I$(top_srcdir)/kabc

noinst_LTLIBRARIES = libvcards.la

libvcards_la_SOURCES = vcard.cpp vcardline.cpp vcardparser.cpp
libvcards_la_LIBADD = $(LIB_KDECORE) -L$(top_srcdir)/kabc -lkabc

check_PROGRAMS = testread testwrite

testread_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testread_LDADD = libvcards.la 
testread_SOURCES = testread.cpp

testwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testwrite_LDADD = libvcards.la 
testwrite_SOURCES = testwrite.cpp
