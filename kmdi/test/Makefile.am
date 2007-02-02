check_PROGRAMS = kfourchildren
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kmdi -I.. $(all_includes)

kfourchildren_SOURCES = hello.cpp main.cpp mainwidget.cpp
kfourchildren_LDFLAGS = -no-undefined $(all_libraries)
kfourchildren_LDADD = $(top_builddir)/kmdi/libkmdi.la \
	$(top_builddir)/kutils/libkutils.la

