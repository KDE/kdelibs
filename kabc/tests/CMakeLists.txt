kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${CMAKE_SOURCE_DIR}/kab ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(testlock_SRCS
testlock.cpp
)

kde4_automoc(${testlock_SRCS})

kde4_add_executable(testlock ${testlock_SRCS})

target_link_libraries(testlock  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(geotest_SRCS geotest.cpp)

kde4_automoc(${geotest_SRCS})

kde4_add_executable(geotest ${geotest_SRCS})

target_link_libraries(geotest  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})

########### next target ###############

set(testkabc_SRCS
testkabc.cpp
)

kde4_automoc(${testkabc_SRCS})

kde4_add_executable(testkabc ${testkabc_SRCS})

target_link_libraries(testkabc  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(testkabcdlg_SRCS
testkabcdlg.cpp
)

kde4_automoc(${testkabcdlg_SRCS})

kde4_add_executable(testkabcdlg ${testkabcdlg_SRCS})

target_link_libraries(testkabcdlg  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(testdistlist_SRCS
testdistlist.cpp
)

kde4_automoc(${testdistlist_SRCS})

kde4_add_executable(testdistlist ${testdistlist_SRCS})

target_link_libraries(testdistlist  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(bigread_SRCS
bigread.cpp
)

kde4_automoc(${bigread_SRCS})


message(STATUS "skipping kabc/tests bigread, how to link to a plugin (kabc_file.so) ?")
#kde4_add_executable(bigread ${bigread_SRCS})
#
#target_link_libraries(bigread  ${QT_AND_KDECORE_LIBS} kabc kabc_file )


########### next target ###############

set(bigwrite_SRCS
bigwrite.cpp
)

kde4_automoc(${bigwrite_SRCS})


message(STATUS "skipping kabc/tests bigwrite, how to link to a plugin (kabc_file.so) ?")

#kde4_add_executable(bigwrite ${bigwrite_SRCS})
#
#target_link_libraries(bigwrite  ${QT_AND_KDECORE_LIBS} kabc kabc_file )


########### next target ###############

set(testdb_SRCS
testdb.cpp
)

kde4_automoc(${testdb_SRCS})


message(STATUS "skipping kabc/tests testdb, mysql check missing")

#kde4_add_executable(testdb ${testdb_SRCS})
#
#target_link_libraries(testdb  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(testaddressee_SRCS
testaddressee.cpp
)

kde4_automoc(${testaddressee_SRCS})


kde4_add_executable(testaddressee ${testaddressee_SRCS})

target_link_libraries(testaddressee  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(testaddresseelist_SRCS
testaddresseelist.cpp
)

kde4_automoc(${testaddresseelist_SRCS})


message(STATUS "skipping kabc/tests testaddresseelist")

#kde4_add_executable(testaddresseelist ${testaddresseelist_SRCS})
#
#target_link_libraries(testaddresseelist  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(testaddressfmt_SRCS
testaddressfmt.cpp
)

kde4_automoc(${testaddressfmt_SRCS})

kde4_add_executable(testaddressfmt ${testaddressfmt_SRCS})

target_link_libraries(testaddressfmt  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(kabcargl_SRCS
kabcargl.cpp
)

kde4_automoc(${kabcargl_SRCS})

kde4_add_executable(kabcargl ${kabcargl_SRCS})

target_link_libraries(kabcargl  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})


########### next target ###############

set(testaddresslineedit_SRCS
testaddresslineedit.cpp
)

kde4_automoc(${testaddresslineedit_SRCS})

kde4_add_executable(testaddresslineedit ${testaddresslineedit_SRCS})

target_link_libraries(testaddresslineedit  ${QT_AND_KDECORE_LIBS} kabc ${QT_QTTEST_LIBRARY})

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

## Make sure $(all_includes) remains last!
#INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_srcdir)/kab \
#           -I$(srcdir)/../vcardparser/ -I$(srcdir)/../vcard/include \
#           -I$(srcdir)/../vcard/include/generated \
#           -I$(srcdir)/../vcardparser $(all_includes)
#LDADD = ../libkabc.la -l${QT_QTTEST_LIBRARY}_debug
#
#METASOURCES = AUTO
#
#check_PROGRAMS = testlock geotest
#
#geotest_SOURCES = geotest.cpp
#
#testlock_LDFLAGS = $(all_libraries)
#testlock_SOURCES = testlock.cpp
#
#EXTRA_PROGRAMS = testkabc testkabcdlg testdistlist bigread bigwrite testdb \
#  testaddressee testaddresseelist testaddressfmt kabcargl testaddresslineedit
#
#testkabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testkabc_SOURCES = testkabc.cpp
#
#testaddressee_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testaddressee_SOURCES = testaddressee.cpp
#
#testaddresseelist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testaddresseelist_SOURCES = testaddresseelist.cpp
#
#testaddressfmt_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testaddressfmt_SOURCES = testaddressfmt.cpp
#
#testkabcdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testkabcdlg_SOURCES = testkabcdlg.cpp
#
#testdistlist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testdistlist_SOURCES = testdistlist.cpp
#
#testaddresslineedit_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testaddresslineedit_SOURCES = testaddresslineedit.cpp
#
#bigread_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#bigread_LDADD = ../libkabc.la $(top_builddir)/kabc/plugins/file/libkabc_file.la
#bigread_SOURCES = bigread.cpp
#
#bigwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#bigwrite_LDADD = ../libkabc.la $(top_builddir)/kabc/plugins/file/libkabc_file.la
#bigwrite_SOURCES = bigwrite.cpp
#
#testdb_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testdb_SOURCES = testdb.cpp
#
#kabcargl_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kabcargl_SOURCES = kabcargl.cpp
#
