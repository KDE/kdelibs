CUPSSUBDIR = @CUPSSUBDIR@
LIB_KDEPRINT_CUPS = @LIB_KDEPRINT_CUPS@
LIBVERSION = 1:0

SUBDIRS = $(CUPSSUBDIR) pdq . standalone pics

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kdeprint/cups -I$(top_srcdir)/kdeprint/pdq -I$(top_srcdir)/kfile -I$(top_srcdir)/kio $(all_includes)


UIFILES = kprintdialogbase.ui kprintcopiespagebase.ui kprintsizecolorpagebase.ui
HFILES = $(UIFILES:.ui=.h)

uicfiles: 
	rm -f $(srcdir)/kprintdialogbase.cpp
	$(UIC) -tr i18n $(srcdir)/kprintdialogbase.ui > $(srcdir)/kprintdialogbase.h
	echo '#include <klocale.h>' > $(srcdir)/kprintdialogbase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kprintdialogbase.h $(srcdir)/kprintdialogbase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kprintdialogbase.cpp || rm -f $(srcdir)/kprintdialogbase.cpp
	rm -f $(srcdir)/kprintcopiespagebase.cpp
	$(UIC) -tr i18n $(srcdir)/kprintcopiespagebase.ui > $(srcdir)/kprintcopiespagebase.h
	echo '#include <klocale.h>' > $(srcdir)/kprintcopiespagebase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kprintcopiespagebase.h $(srcdir)/kprintcopiespagebase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kprintcopiespagebase.cpp || rm -f $(srcdir)/kprintcopiespagebase.cpp
	rm -f $(srcdir)/kprintsizecolorpagebase.cpp
	$(UIC) -tr i18n $(srcdir)/kprintsizecolorpagebase.ui > $(srcdir)/kprintsizecolorpagebase.h
	echo '#include <klocale.h>' > $(srcdir)/kprintsizecolorpagebase.cpp
	$(UIC) -tr i18n -i $(srcdir)/kprintsizecolorpagebase.h $(srcdir)/kprintsizecolorpagebase.ui | sed -e "s,i18n( \"\" ),QString::null,g" >> $(srcdir)/kprintsizecolorpagebase.cpp || rm -f $(srcdir)/kprintdsizecolorpagebase.cpp

lib_LTLIBRARIES = libkdeprint.la
libkdeprint_la_SOURCES = kdialogimpl.cpp kprintcopiespage.cpp kprintdialog.cpp kprinter.cpp kprinteritem.cpp \
			 optionset.cpp kprintdialogbase.cpp kprintcopiespagebase.cpp kprintdialogpage.cpp \
			 kprinterpropertydialog.cpp kprintsizecolorpagebase.cpp kprintsizecolorpage.cpp \
			 klpddialogimpl.cpp klpdprinter.cpp previewshell.cpp kjoblister.cpp

libkdeprint_la_METASOURCES = AUTO
libkdeprint_la_LDFLAGS = -version-info $(LIBVERSION) $(all_libraries)
libkdeprint_la_LIBADD = $(LIB_KDEPRINT_CUPS) $(top_builddir)/kdeprint/pdq/libkdeprint_pdq.la $(LIB_KPARTS) $(LIB_KFILE)

noinst_HEADERS = kprintcopiespage.h kprintdialog.h \
		 kprinterpropertydialog.h kprintsizecolorpage.h klpddialogimpl.h klpdprinter.h \
		 previewshell.h
include_HEADERS = kprinter.h


kdeprint_HEADERS = optionset.h kjob.h kjoblister.h kprinteritem.h kdialogimpl.h kprintdialogpage.h
kdeprintdir = $(includedir)/kdeprint

uidata_DATA = previewshellui.rc
uidatadir = $(kde_confdir)/ui

rcdata_DATA = kdeprintrc
rcdatadir = $(kde_confdir)

