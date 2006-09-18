SUBDIRS = knewstuff2

bin_PROGRAMS = kdxspreview

AM_CPPFLAGS = $(all_includes)

kdxspreview_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kdxspreview_LDADD = $(LIB_KFILE) $(LIB_KHTML) knewstuff2/libknewstuff2.la
kdxspreview_SOURCES = main.cpp \
	kdxs.cpp \
	kdxsview.cpp \
	kdxsbutton.cpp \
	kdxsrating.cpp \
	kdxscomment.cpp \
	kdxscomments.cpp \
	kdxschanges.cpp \
	kdxstranslation.cpp \
	soap.cpp \
	dxs.cpp \
	newstuff.cpp \
	qasyncpixmap.cpp \
	qasyncframe.cpp \
	qstarframe.cpp \
	qnowtooltip.cpp

noinst_HEADERS = \
	kdxs.h \
	kdxsview.h \
	kdxsbutton.h \
	kdxsrating.h \
	kdxscomment.h \
	kdxscomments.h \
	kdxschanges.h \
	kdxstranslation.h \
	soap.h \
	dxs.h \
	newstuff.h \
	qasyncpixmap.h \
	qasyncframe.h \
	qstarframe.h \
	qnowtooltip.h

METASOURCES = AUTO
