
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/libltdl -I$(top_srcdir)/kfile -I$(top_srcdir)/kio $(all_includes)
AM_LDFLAGS = $(all_libraries)
LDADD = $(LIB_KPARTS)

check_PROGRAMS = kpartstest normalktmtest ghostviewtest 

noinst_LTLIBRARIES = libnotepad.la
libnotepad_la_SOURCES = notepad.cpp

kpartstest_SOURCES = example.cpp parts.cpp
kpartstest_LDADD = libnotepad.la
normalktmtest_SOURCES = normalktm.cpp parts.cpp
normalktmtest_LDADD = libnotepad.la
ghostviewtest_SOURCES = ghostview.cpp
ghostviewtest_LDADD = $(LIB_KPARTS) 

# These are not really libraries, but modules dynamically opened.
# So they should be installed in kde_module_dir, which is usually $prefix/lib/kde2
kde_module_LTLIBRARIES = libspellcheckplugin.la libnotepadpart.la

libspellcheckplugin_la_SOURCES = plugin_spellcheck.cpp
libspellcheckplugin_la_LIBADD = $(LIB_KPARTS) 
libspellcheckplugin_la_LDFLAGS = -module $(KDE_PLUGIN)

libnotepadpart_la_SOURCES = notepad_factory.cpp
libnotepadpart_la_LIBADD = $(LIB_KPARTS) libnotepad.la 
libnotepadpart_la_LDFLAGS = -module $(KDE_PLUGIN)

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

