
INCLUDES= -I$(includedir)/arts -I$(top_srcdir)/arts/kde $(all_includes)

bin_PROGRAMS = kmcop
lib_LTLIBRARIES = kmcop.la

kmcop_la_SOURCES = mcopdcopobject.cpp kmcop.cpp kmcop.skel
kmcop_la_LIBADD	= ../libartskde.la
kmcop_la_LDFLAGS = $(all_libraries) -module -avoid-version
kmcop_la_METASOURCES = AUTO

kmcop_SOURCES = dummy.cpp
kmcop_LDADD = kmcop.la
kmcop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

noinst_HEADERS = kmcop.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = kmcop.desktop

messages:
	$(XGETTEXT) kmcop.cpp -o $(podir)/kmcop.pot

dummy.cpp:
	echo > dummy.cpp
