kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libltdl ${CMAKE_SOURCE_DIR}/kfile ${CMAKE_SOURCE_DIR}/kio ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kpartstest_SRCS
example.cpp
parts.cpp
)

kde4_automoc(${kpartstest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kpartstest ${kpartstest_SRCS})

target_link_libraries(kpartstest  ${QT_AND_KDECORE_LIBS} kparts )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(normalktmtest_SRCS
normalktm.cpp
parts.cpp
)

kde4_automoc(${normalktmtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(normalktmtest ${normalktmtest_SRCS})

target_link_libraries(normalktmtest  ${QT_AND_KDECORE_LIBS} kparts )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ghostviewtest_SRCS
ghostview.cpp
)

kde4_automoc(${ghostviewtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ghostviewtest ${ghostviewtest_SRCS})

target_link_libraries(ghostviewtest  ${QT_AND_KDECORE_LIBS} kparts )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(spellcheckplugin_PART_SRCS
plugin_spellcheck.cpp
)

kde4_automoc(${spellcheckplugin_PART_SRCS})

kde4_add_plugin(spellcheckplugin WITH_PREFIX ${spellcheckplugin_PART_SRCS})

target_link_libraries(spellcheckplugin  ${QT_AND_KDECORE_LIBS} kparts )

install_targets(/lib/kde4 spellcheckplugin )


########### next target ###############

set(notepadpart_PART_SRCS
notepadpart.cpp
)

kde4_automoc(${notepadpart_PART_SRCS})

kde4_add_plugin(notepadpart WITH_PREFIX ${notepadpart_PART_SRCS})

target_link_libraries(notepadpart  ${QT_AND_KDECORE_LIBS} kparts )

install_targets(/lib/kde4 notepadpart )


########### next target ###############


########### install files ###############

install_files( ${KDE4_DATA_DIR}/notepadpart/kpartplugins FILES plugin_foobar.rc plugin_spellcheck.rc )
install_files( ${KDE4_DATA_DIR}/notepadpart FILES notepadpart.rc )
install_files( ${KDE4_DATA_DIR}/ghostviewtest FILES ghostviewtest_shell.rc )
install_files( ${KDE4_DATA_DIR}/kpartstest FILES kpartstest_shell.rc )
install_files( ${KDE4_DATA_DIR}/kpartstestpart FILES kpartstest_part1.rc )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/libltdl -I$(top_srcdir)/kfile -I$(top_srcdir)/kio $(all_includes)
#AM_LDFLAGS = $(all_libraries)
#LDADD = $(LIB_KPARTS)
#
#check_PROGRAMS = kpartstest normalktmtest ghostviewtest
#
#kpartstest_SOURCES = example.cpp parts.cpp
#kpartstest_LDADD = libnotepad.la $(LIB_KPARTS)
#normalktmtest_SOURCES = normalktm.cpp parts.cpp
#normalktmtest_LDADD = libnotepad.la $(LIB_KPARTS)
#ghostviewtest_SOURCES = ghostview.cpp
#ghostviewtest_LDADD = $(LIB_KPARTS)
#
## These are not really libraries, but modules dynamically opened.
## So they should be installed in kde_module_dir, which is usually $prefix/lib/kde3
#kde_module_LTLIBRARIES = libspellcheckplugin.la libnotepadpart.la
#
#libspellcheckplugin_la_SOURCES = plugin_spellcheck.cpp
#libspellcheckplugin_la_LIBADD = $(LIB_KPARTS) 
#libspellcheckplugin_la_LDFLAGS = -module $(KDE_PLUGIN)
#
#libnotepadpart_la_SOURCES = notepadpart.cpp
#libnotepadpart_la_LIBADD = libnotepad.la $(LIB_KPARTS)
#libnotepadpart_la_LDFLAGS = -module $(KDE_PLUGIN)
#
#noinst_LTLIBRARIES = libnotepad.la
#libnotepad_la_SOURCES = notepad.cpp
#
#rcdir = $(kde_datadir)/kpartstest
#rc_DATA = kpartstest_shell.rc
#
#testpartdir = $(kde_datadir)/kpartstestpart
#testpart_DATA = kpartstest_part1.rc
#
#notepaddir = $(kde_datadir)/notepadpart
#notepad_DATA = notepadpart.rc
#
#ghostviewtestdir = $(kde_datadir)/ghostviewtest
#ghostviewtest_DATA = ghostviewtest_shell.rc
#
#pluginsdir = $(kde_datadir)/notepadpart/kpartplugins
#plugins_DATA = plugin_foobar.rc plugin_spellcheck.rc
#
#METASOURCES = AUTO
#
