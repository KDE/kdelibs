INCLUDES = $(all_includes) -I$(top_builddir)/kabc -I$(top_srcdir)/kabc

noinst_LTLIBRARIES = libvcards.la

libvcards_la_SOURCES = vcard.cpp vcardline.cpp vcardparser.cpp vcardtool.cpp
libvcards_la_LIBADD = $(LIB_KDECORE) -L$(top_srcdir)/kabc -lkabc

check_PROGRAMS = testread testwrite

testread_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testread_LDADD = libvcards.la 
testread_SOURCES = testread.cpp

testwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testwrite_LDADD = libvcards.la 
testwrite_SOURCES = testwrite.cpp

check: testread
	$(top_builddir)/kabc/vcardparser/testread $(top_srcdir)/kabc/vcardparser/tests/vcard1.vcf dummy.out
	$(top_builddir)/kabc/vcardparser/testread $(top_srcdir)/kabc/vcardparser/tests/vcard2.vcf dummy.out
	$(top_builddir)/kabc/vcardparser/testread $(top_srcdir)/kabc/vcardparser/tests/vcard3.vcf dummy.out
	
