LIB_CUPS = @LIB_CUPS@
LIBVERSION = @LIBVERSION@
INCLUDES = -I.. $(all_includes)

lib_LTLIBRARIES = libkdeprint_cups.la

libkdeprint_cups_la_SOURCES = cupsinfos.cpp ipprequest.cpp kcupsdialogimpl.cpp kcupsgeneralpage.cpp kcupsppdpage.cpp \
			      kcupsprinter.cpp kppdbaseitem.cpp kppdgroupitem.cpp kppdnumericaloptionitem.cpp \
			      kppdoptionitem.cpp kppdoptionview.cpp kppdview.cpp kppdwidget.cpp numericwidget.cpp \
			      kcupsgeneralpagebase.ui globalppd.c icons.cpp kcupsimagepagebase.ui kcupsimagepage.cpp \
			      previewimage.cpp position.cpp marginpreview.cpp image.cpp kcupstextpagebase.ui \
			      kcupstextpage.cpp qclineedit.cpp columnpreview.cpp kcupshpgl2pagebase.ui kcupshpgl2page.cpp \
			      kcupsoptiondlgbase.ui kcupsoptiondlg.cpp kcupsjoblister.cpp
libkdeprint_cups_la_METASOURCES = AUTO
libkdeprint_cups_la_LDFLAGS = -version-info $(LIBVERSION) $(all_libraries)
libkdeprint_cups_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_CUPS) $(LIBZ)

pkginclude_HEADERS = cupsinfos.h globalppd.h ipprequest.h kppdwidget.h
pkgincludedir = $(includedir)/kdeprint/cups

noinst_HEADERS = kcupsdialogimpl.h kcupsgeneralpage.h kcupsppdpage.h kcupsprinter.h kppdbaseitem.h \
		 kppdgroupitem.h kppdnumericaloptionitem.h kppdoptionitem.h kppdoptionview.h kppdview.h\
		 numericwidget.h ppd-util.h icons.h kcupsimagepage.h previewimage.h position.h marginpreview.h \
		 kcupstextpage.h qclineedit.h columnpreview.h kcupshpgl2page.h kcupsoptiondlg.h kcupsjoblister.h
