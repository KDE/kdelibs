INCLUDES = -I$(top_srcdir)/kdeprint -I$(top_srcdir) -I$(top_builddir)/kdeprint $(all_includes)
LIBVERSION = 1:0

####### Files

lib_LTLIBRARIES = libkdeprint_pdq.la

libkdeprint_pdq_la_SOURCES = parser.cpp scanner.cpp pdq.cpp pdqclass.cpp pdqinfos.cpp kpdqdialogimpl.cpp \
			     kpdqprinter.cpp kpdqbaseitem.cpp kpdqoptionitem.cpp kpdqargitem.cpp kpdqview.cpp \
			     kpdqoptionview.cpp kpdqpage.cpp kpdqoptiondlg.cpp kpdqjoblister.cpp
libkdeprint_pdq_la_METASOURCES = AUTO

noinst_HEADERS = parser.cpp.h pdq.h pdqclass.h kpdqdialogimpl.h kpdqprinter.h kpdqbaseitem.h kpdqoptionitem.h \
		 kpdqargitem.h kpdqview.h kpdqoptionview.h kpdqpage.h kpdqoptiondlg.h kpdqjoblister.h

libkdeprint_pdq_la_LIBADD = $(LIB_KDEUI)
libkdeprint_pdq_la_LDFLAGS = -version-info $(LIBVERSION) $(all_libraries)

####### Build rules

parser.cpp: parser.yy
	cd $(srcdir) && bison -t -d -v -o parser.cpp parser.yy

scanner.cpp: scanner.ll parser.cpp
	cd $(srcdir) && flex -B -8 -oscanner.cpp scanner.ll

EXTRA_DIST = scanner.ll parser.yy

CLEANFILES = parser.cpp scanner.cpp parser.cpp.h parser.cpp.output
