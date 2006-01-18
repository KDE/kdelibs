kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(testread_SRCS
testread.cpp
)

kde4_automoc(${testread_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testread ${testread_SRCS})

target_link_libraries(testread  ${QT_AND_KDECORE_LIBS} kabc )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testwrite_SRCS
testwrite.cpp
)

kde4_automoc(${testwrite_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testwrite ${testwrite_SRCS})

target_link_libraries(testwrite  ${QT_AND_KDECORE_LIBS} kabc )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testread2_SRCS
testread2.cpp
testutils.cpp
)

kde4_automoc(${testread2_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testread2 ${testread2_SRCS})

target_link_libraries(testread2  ${QT_AND_KDECORE_LIBS} kabc )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)
#
#noinst_LTLIBRARIES = libvcards.la
#libvcards_la_SOURCES = vcard.cpp vcardline.cpp vcardparser.cpp
#
#vcardsincludedir = $(includedir)/kabc
#vcardsinclude_HEADERS = vcard.h vcardline.h vcardparser.h
#
#check_PROGRAMS = testread testwrite testread2
#
#testread_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testread_LDADD = libvcards.la $(top_builddir)/kabc/libkabc.la
#testread_SOURCES = testread.cpp
#
#testread2_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testread2_LDADD = libvcards.la $(top_builddir)/kabc/libkabc.la
#testread2_SOURCES = testread2.cpp testutils.cpp
#
#testwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testwrite_LDADD = libvcards.la $(top_builddir)/kabc/libkabc.la
#testwrite_SOURCES = testwrite.cpp
#
#TESTFILES = vcard1.vcf vcard2.vcf vcard3.vcf vcard4.vcf vcard6.vcf vcard7.vcf
#
#check-local: testread
#	rm -f FAILED; \
#	for i in $(TESTFILES); \
#	  do perl $(top_srcdir)/kabc/vcardparser/checkvcard.pl \
#	    $(top_srcdir)/kabc/vcardparser/tests/$$i; \
#	done; \
#	[ ! -e FAILED ]
