
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/libltdl -I$(top_srcdir)/kfile -I$(top_srcdir)/kio $(all_includes)
LDFLAGS	= $(all_libraries)
LDADD = $(top_builddir)/kparts/libkparts.la $(LIB_KDECORE)

check_PROGRAMS = kpartstest normalktmtest ghostviewtest 

kpartstest_SOURCES = example.cpp notepad.cpp parts.cpp
normalktmtest_SOURCES = normalktm.cpp notepad.cpp parts.cpp
ghostviewtest_SOURCES = ghostview.cpp
ghostviewtest_LDADD = $(top_builddir)/kparts/libkparts.la $(LIB_KFILE)

lib_LTLIBRARIES = libspellcheck.la libnotepad.la

libspellcheck_la_SOURCES = plugin_spellcheck.cpp
libspellcheck_la_LIBADD = $(top_builddir)/kparts/libkparts.la $(LIB_KDEUI)
libspellcheck_la_LDFLAGS = -avoid-version -module -no-undefined

libnotepad_la_SOURCES = notepad.cpp notepad_factory.cpp
libnotepad_la_LIBADD = $(top_builddir)/kparts/libkparts.la $(LIB_KDEUI)
libnotepad_la_LDFLAGS = -avoid-version -module -no-undefined

rcdir = $(kde_datadir)/kpartstest
rc_DATA = kpartstest_shell.rc

testpartdir = $(kde_datadir)/kpartstestpart
testpart_DATA = kpartstest_part1.rc

notepaddir = $(kde_datadir)/notepadpart
notepad_DATA = notepadpart.rc

ghostviewtestdir = $(kde_datadir)/ghostviewtest
ghostviewtest_DATA = ghostviewtest_shell.rc

pluginsdir = $(kde_datadir)/notepadpart/kpartplugins
plugins_DATA = plugin_foobar.rc plugin_spellcheck.rc

METASOURCES = AUTO

