
INCLUDES= $(all_includes)
LDADD	= $(LIB_QT) ../kdecore/libkdecore.la -lqk
LDFLAGS = $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS = ksycoca

ksycoca_SOURCES = \
	ksycoca.cpp ksycoca.h ksycocafactory.h \
	ksycocadict.cpp ksycocadict.h ksycocaentry.cpp ksycocaentry.h \
        kservicetypefactory.h kservicetypefactory.cpp \
	kservicetype.cpp kservicetype.h main.cpp

ksycoca_METASOURCES = USE_AUTOMOC

noinst_HEADERS =

EXTRA_DIST =

