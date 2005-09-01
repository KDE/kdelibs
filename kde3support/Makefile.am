SUBDIRS=kdeui kparts kmdi .
lib_LTLIBRARIES=libkde3support.la

libkde3support_la_SOURCES = dummy.cpp

libkde3support_la_LDFLAGS = -no-undefined $(all_libraries) \
        $(KDE_MT_LDFLAGS)
libkde3support_la_LIBADD = ./kmdi/libk3mdi.la ./kparts/libk3parts.la ./kdeui/libkde3ui.la

dummy.cpp:
	echo >dummy.cpp
