
INCLUDES= -I$(top_srcdir)/kparts $(all_includes)
LDFLAGS	= $(all_libraries)
LDADD = $(top_builddir)/kparts/libkparts.la $(LIB_KDECORE)

check_PROGRAMS = kpartstest

kpartstest_SOURCES = example.cpp notepad.cpp

lib_LTLIBRARIES = libspellcheck.la
libspellcheck_la_LDFLAGS = -avoid-version -module
libspellcheck_la_LIBADD = $(top_builddir)/kparts/libkparts.la $(LIB_KDEUI)
libspellcheck_la_SOURCES = plugin_spellcheck.cpp

rcdir = $(kde_datadir)/kpartstest
rc_DATA = kpartstest_shell.rc

testpartdir = $(kde_datadir)/kpartstestpart
testpart_DATA = kpartstest_part1.rc

notepaddir = $(kde_datadir)/notepadpart
notepad_DATA = notepadpart.rc

pluginsdir = $(kde_datadir)/notepadpart/kpartplugins
plugins_DATA = plugin_foobar.rc plugin_spellcheck.rc

METASOURCES = AUTO

