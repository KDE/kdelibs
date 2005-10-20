check_PROGRAMS = kfourchildren
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kde3support/kmdi -I$(top_srcdir)/kde3support/kparts \
	   -I$(top_srcdir)/kde3support/kdeui -I$(top_srcdir)/kde3support -I.. $(all_includes)

kfourchildren_SOURCES = hello.cpp main.cpp mainwidget.cpp
kfourchildren_LDFLAGS = -no-undefined $(all_libraries)
kfourchildren_LDADD =   ../../../kutils/libkutils.la ../../libkde3support.la

