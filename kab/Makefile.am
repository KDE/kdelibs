# set the include path for X, qt and KDE
# set "REMOVE_NANA" to get rid of the NANA assertions:
# (REMOVE_NANA is now defined by default if the compiler directives
#  do not define DEBUG), I use MS_NANA_DEFINES for this:
INCLUDES= -I$(top_srcdir)/kfile -I$(top_srcdir)/kio -I$(top_srcdir)/nana $(all_includes) 
# -DKAB_DEBUG

lib_LTLIBRARIES = libkab.la
KABVERSION=	3.0.0
KABPATCH=	0

include_HEADERS = \
	kabapi.h \
	addressbook.h \
	qconfigDB.h 
# 	kab.h \
# 	businesscard.h \
# 	StringListEditDialog.h \
# 	StringListEditWidget.h \
# 	StringListEditWidgetData.h

# Which sources should be compiled for khexdit.
libkab_la_SOURCES	= \
	kabapi.cc \
	addressbook.cc \
	qconfigDB.cc 
#	editentry.cc \
#	kab.cc \
#	businesscard.cc \
#	StringListEditDialog.cpp \
#	StringListEditWidget.cpp \
#	StringListEditWidgetData.cpp

libkab_la_LDFLAGS = -version-info 3:0 $(X_LDFLAGS) $(QT_LDFLAGS) $(USER_LDFLAGS) -no-undefined
libkab_la_LIBADD = $(LIB_KDEUI) ../nana/libnana.la

libkab_la_METASOURCES = AUTO

picsdir = $(kde_datadir)/kab/pics
pics_DATA = 
# dialog_background.jpg background_1.jpg

# dlgfiles = AssertDialog.dlg PrintDialog.dlg \
#	StringListEditWidget.dlg StringListSelectAndReorderSet.dlg \
#	StringListSelectOneDialog.dlg StringListSelectSetDialog.dlg
# DialogBase.dlg 

# noinst_HEADERS = AssertDialog.h AssertDialogData.h \
#	PrintDialog.h PrintDialogData.h SearchDialog.h \
#	SearchDialogMainWidget.h SearchDialogMainWidgetData.h \
#	StringListEditDialog.h StringListEditWidget.h \
#	StringListEditWidgetData.h StringListSelectAndReorderSet.h \
#	StringListSelectAndReorderSetData.h StringListSelectOneDialog.h \
#	StringListSelectOneDialogData.h StringListSelectSetDialog.h \
#	arrow_down.xbm arrow_left.xbm arrow_right.xbm arrow_up.xbm \
#	bug_3d.xpm businesscard.h datepickerdialog.h dbarr_next.xbm \
#	dbarr_previous.xbm debug.h editentry.h \
#	magnify2.xbm searchresults.h select.xbm text.xbm trash_open.xbm \
#	widgets.h

# EXTRA_DIST = $(pics_DATA) $(dlgfiles) COMPILE_PROBLEMS_FAQ README.FORMAT \
#	README.KABAPI kab.prj kabapi.h.kdoc libkab.kdoc msui.prj 

