
INCLUDES= -I$(top_srcdir)/kparts $(all_includes)
LDFLAGS	= $(all_libraries)
LDADD = $(top_builddir)/kparts/libkparts.la $(LIB_KDECORE)

check_PROGRAMS = kpartstest

kpartstest_SOURCES = example.cpp notepad.cpp

rcdir = $(kde_datadir)/kpartstest
rc_DATA = kpartstest_shell.rc

testpartdir = $(kde_datadir)/kpartstestpart
testpart_DATA = kpartstest_part1.rc

notepaddir = $(kde_datadir)/notepadpart
notepad_DATA = notepadpart.rc

METASOURCES = AUTO

