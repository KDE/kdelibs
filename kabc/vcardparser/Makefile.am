INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)

lib_LTLIBRARIES = libvcards.la

libvcards_la_SOURCES = vcard.cpp vcardline.cpp vcardparser.cpp

vcardsincludedir = $(includedir)/kabc
vcardsinclude_HEADERS = vcard.h vcardline.h vcardparser.h

check_PROGRAMS = testread testwrite testread2

testread_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testread_LDADD = libvcards.la ../libkabc.la
testread_SOURCES = testread.cpp

testread2_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testread2_LDADD = libvcards.la ../libkabc.la 
testread2_SOURCES = testread2.cpp testutils.cpp

testwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testwrite_LDADD = libvcards.la ../libkabc.la
testwrite_SOURCES = testwrite.cpp

TESTFILES = vcard1.vcf vcard2.vcf vcard3.vcf vcard4.vcf vcard5.vcf

check: testread
	for i in $(TESTFILES); do perl $(top_srcdir)/kabc/vcardparser/checkvcard.pl \
          $(top_srcdir)/kabc/vcardparser/tests/$$i; done
