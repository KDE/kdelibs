bin_PROGRAMS = kdxspreview

AM_CPPFLAGS = $(all_includes)

kdxspreview_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kdxspreview_LDADD = $(LIB_KFILE) $(LIB_KHTML) $(LIB_KNEWSTUFF)
kdxspreview_SOURCES = main.cpp \
	kdxs.cpp \
	kdxsview.cpp \
	kdxsbutton.cpp \
	kdxsrating.cpp \
	kdxscomment.cpp \
	kdxscomments.cpp \
	soap.cpp \
	dxs.cpp \
	kns-translation.ui \
	newstuff.cpp \
	qasyncpixmap.cpp \
	qasyncframe.cpp

noinst_HEADERS = \
	kdxs.h \
	kdxsview.h \
	kdxsbutton.h \
	kdxsrating.h \
	kdxscomment.h \
	kdxscomments.h \
	soap.h \
	dxs.h \
	newstuff.h \
	qasyncpixmap.h \
	qasyncframe.h

METASOURCES = AUTO
