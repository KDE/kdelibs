LIB_CUPS = @LIB_CUPS@
LIBVERSION = 1:0
INCLUDES = -I$(top_srcdir)/kdeprint -I$(top_srcdir) -I$(top_builddir)/kdeprint $(all_includes)

lib_LTLIBRARIES = libkdeprint_cups.la

uicfiles: 
	rm -f $(srcdir)/kcupsgeneralpagebase.cpp
	$(UIC) -tr i18n $(srcdir)/kcupsgeneralpagebase.ui > $(srcdir)/kcupsgeneralpagebase.h
	echo '#include <klocale.h>' > $(srcdir)/kcupsgeneralpagebase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kcupsgeneralpagebase.h $(srcdir)/kcupsgeneralpagebase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kcupsgeneralpagebase.cpp || rm -f $(srcdir)/kcupsgeneralpagebase.cpp
	rm -f $(srcdir)/kcupsimagepagebase.cpp
	$(UIC) -tr i18n $(srcdir)/kcupsimagepagebase.ui > $(srcdir)/kcupsimagepagebase.h
	echo '#include <klocale.h>' > $(srcdir)/kcupsimagepagebase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kcupsimagepagebase.h $(srcdir)/kcupsimagepagebase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kcupsimagepagebase.cpp || rm -f $(srcdir)/kcupsimagepagebase.cpp
	rm -f $(srcdir)/kcupstextpagebase.cpp
	$(UIC) -tr i18n $(srcdir)/kcupstextpagebase.ui > $(srcdir)/kcupstextpagebase.h
	echo '#include <klocale.h>' > $(srcdir)/kcupstextpagebase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kcupstextpagebase.h $(srcdir)/kcupstextpagebase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kcupstextpagebase.cpp || rm -f $(srcdir)/kcupstextpagebase.cpp
	rm -f $(srcdir)/kcupshpgl2pagebase.cpp
	$(UIC) -tr i18n $(srcdir)/kcupshpgl2pagebase.ui > $(srcdir)/kcupshpgl2pagebase.h
	echo '#include <klocale.h>' > $(srcdir)/kcupshpgl2pagebase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kcupshpgl2pagebase.h $(srcdir)/kcupshpgl2pagebase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kcupshpgl2pagebase.cpp || rm -f $(srcdir)/kcupshpgl2pagebase.cpp
	rm -f $(srcdir)/kcupsoptiondlgbase.cpp
	$(UIC) -tr i18n $(srcdir)/kcupsoptiondlgbase.ui > $(srcdir)/kcupsoptiondlgbase.h
	echo '#include <klocale.h>' > $(srcdir)/kcupsoptiondlgbase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kcupsoptiondlgbase.h $(srcdir)/kcupsoptiondlgbase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kcupsoptiondlgbase.cpp || rm -f $(srcdir)/kcupsoptiondlgbase.cpp


libkdeprint_cups_la_SOURCES = cupsinfos.cpp ipprequest.cpp kcupsdialogimpl.cpp kcupsgeneralpage.cpp kcupsppdpage.cpp \
			      kcupsprinter.cpp kppdbaseitem.cpp kppdgroupitem.cpp kppdnumericaloptionitem.cpp \
			      kppdoptionitem.cpp kppdoptionview.cpp kppdview.cpp kppdwidget.cpp numericwidget.cpp \
			      kcupsgeneralpagebase.cpp globalppd.c icons.cpp kcupsimagepagebase.cpp kcupsimagepage.cpp \
			      previewimage.cpp position.cpp marginpreview.cpp image.cpp kcupstextpagebase.cpp \
			      kcupstextpage.cpp qclineedit.cpp columnpreview.cpp kcupshpgl2pagebase.cpp kcupshpgl2page.cpp \
			      kcupsoptiondlgbase.cpp kcupsoptiondlg.cpp kcupsjoblister.cpp
libkdeprint_cups_la_METASOURCES = AUTO
libkdeprint_cups_la_LDFLAGS = -version-info $(LIBVERSION) $(all_libraries)
libkdeprint_cups_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_CUPS) $(LIBZ)

pkginclude_HEADERS = cupsinfos.h globalppd.h ipprequest.h kppdwidget.h
pkgincludedir = $(includedir)/kdeprint/cups

noinst_HEADERS = kcupsdialogimpl.h kcupsgeneralpage.h kcupsppdpage.h kcupsprinter.h kppdbaseitem.h \
		 kppdgroupitem.h kppdnumericaloptionitem.h kppdoptionitem.h kppdoptionview.h kppdview.h\
		 numericwidget.h ppd-util.h icons.h kcupsimagepage.h previewimage.h position.h marginpreview.h \
		 kcupstextpage.h qclineedit.h columnpreview.h kcupshpgl2page.h kcupsoptiondlg.h kcupsjoblister.h
