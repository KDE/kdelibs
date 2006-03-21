INCLUDES = $(all_includes)

SUBDIRS = . ifaces

lib_LTLIBRARIES  = libkdehw.la 
libkdehw_la_LIBADD = $(LIB_QTCORE) $(LIB_QT) $(LIB_KDECORE) $(LIB_KIO) ifaces/libkdehwifaces.la
libkdehw_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)

include_HEADERS = device.h devicemanager.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h display.h predicate.h

METASOURCES = AUTO

libkdehw_la_SOURCES = device.cpp devicemanager.cpp capability.cpp processor.cpp block.cpp storage.cpp cdrom.cpp volume.cpp opticaldisc.cpp display.cpp predicate.cpp predicateparse.cpp predicate_lexer.c predicate_parser.c


kde_servicetypes_DATA = kdehwdevicemanager.desktop


parserfiles = predicate_parser.y predicate_lexer.l

EXTRA_DIST = $(parserfiles)

parser: $(parserfiles)
	cd $(srcdir) ;\
	flex -opredicate_lexer.c predicate_lexer.l ;\
	bison -d predicate_parser.y &&\
	mv predicate_parser.tab.c predicate_parser.c &&\
	mv predicate_parser.tab.h predicate_parser.h

.PHONY: parser


