
# set the include path for X, qt and KDE
# set "REMOVE_NANA" to get rid of the NANA assertions:
# (REMOVE_NANA is now defined by default if the compiler directives
#  do not define DEBUG), I use MS_NANA_DEFINES for this:
INCLUDES= -I$(top_srcdir)/kfile -I$(top_srcdir)/kio $(all_includes)

# $(MS_NANA_DEFINES)
# -DREMOVE_NANA

####### This part is very khexdit specific
# you can add here more. This one gets installed 
lib_LTLIBRARIES = libkab.la
KABVERSION=	0.0.1
KABPATCH=	0

include_HEADERS = \
	stl_headers.h \
	kabapi.h \
	kab.h \
	addressbook.h \
	configDB.h \
	keyvaluemap.h \
	sections.h

# Which sources should be compiled for khexdit.
libkab_la_SOURCES	= \
	kabapi.cc \
	AssertDialog.cpp \
	AssertDialogData.cpp \
	PrintDialog.cpp \
	PrintDialogData.cpp \
	SearchDialog.cpp \
	StringListEditWidget.cpp \
	StringListEditWidgetData.cpp \
	StringListEditDialog.cpp \
	addressbook.cc \
	businesscard.cc \
	datepickerdialog.cc \
	editentry.cc \
	searchresults.cc \
	widget.cc \
	configDB.cc \
	sections.cc \
	keyvaluemap.cc \
	functions.cc \
	StringListSelectOneDialog.cpp \
	StringListSelectOneDialogData.cpp \
	StringListSelectSetDialog.cpp \
	StringListSelectAndReorderSet.cpp \
	StringListSelectAndReorderSetData.cpp \
	widgets.cc \
	SearchDialogMainWidget.cpp \
	SearchDialogMainWidgetData.cpp 

#	DialogBase.cpp \
#	DialogBaseData.cpp \


libkab_la_LDFLAGS = -version-info 3:0 $(X_LDFLAGS) $(QT_LDFLAGS) $(USER_LDFLAGS)
libkab_la_LIBADD  = ../kfile/libkfile.la

libkab_la_METASOURCES = AUTO

picsdir = $(kde_datadir)/kab/pics
pics_DATA = dialog_background.jpg background_1.jpg

dlgfiles = AssertDialog.dlg PrintDialog.dlg \
	StringListEditWidget.dlg StringListSelectAndReorderSet.dlg \
	StringListSelectOneDialog.dlg StringListSelectSetDialog.dlg
# DialogBase.dlg 

noinst_HEADERS = AssertDialog.h AssertDialogData.h \
	PrintDialog.h PrintDialogData.h SearchDialog.h \
	SearchDialogMainWidget.h SearchDialogMainWidgetData.h \
	StringListEditDialog.h StringListEditWidget.h \
	StringListEditWidgetData.h StringListSelectAndReorderSet.h \
	StringListSelectAndReorderSetData.h StringListSelectOneDialog.h \
	StringListSelectOneDialogData.h StringListSelectSetDialog.h \
	arrow_down.xbm arrow_left.xbm arrow_right.xbm arrow_up.xbm \
	bug_3d.xpm businesscard.h datepickerdialog.h dbarr_next.xbm \
	dbarr_previous.xbm debug.h editentry.h functions.h \
	magnify2.xbm searchresults.h select.xbm text.xbm trash_open.xbm \
	widgets.h
# DialogBase.h DialogBaseData.h 

EXTRA_DIST = $(pics_DATA) $(dlgfiles) COMPILE_PROBLEMS_FAQ README.FORMAT \
	README.KABAPI kab.prj kabapi.h.kdoc libkab.kdoc msui.prj 

