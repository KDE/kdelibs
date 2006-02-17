
INCLUDES= -I$(includedir)/arts -I$(top_srcdir)/arts/kde $(all_includes)

bin_PROGRAMS=
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kmcop.la

kmcop_la_SOURCES = mcopdcopobject.cpp kmcop.cpp kmcop.skel
kmcop_la_LIBADD	= ../libartskde.la
kmcop_la_LDFLAGS = $(all_libraries) -module -avoid-version
kmcop_la_METASOURCES = AUTO

noinst_HEADERS = kmcop.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = kmcop.desktop

messages:
	$(XGETTEXT) rc.cpp kmcop.cpp -o $(podir)/kmcop.pot

