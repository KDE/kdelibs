
# set the include path for X, qt and KDE
# set "REMOVE_NANA" to get rid of the NANA assertions:
# (REMOVE_NANA is now defined by default if the compiler directives
#  do not define DEBUG), I use MS_NANA_DEFINES for this:
INCLUDES= -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui -I$(top_srcdir)/kfile -I$(top_srcdir)/kfmlib $(QT_INCLUDES) $(X_INCLUDES)

# $(MS_NANA_DEFINES)
# -DREMOVE_NANA

####### This part is very khexdit specific
# you can add here more. This one gets installed 
lib_LTLIBRARIES = libkab.la
KABVERSION=	0.0.1
KABPATCH=	0

# Which sources should be compiled for khexdit.
libkab_la_SOURCES	= \
	kabapi.cc \
	AssertDialog.cpp \
	AssertDialogData.cpp \
	DialogBase.cpp \
	DialogBaseData.cpp \
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

include_HEADERS = \
	stl_headers.h \
	kabapi.h \
	kab.h \
	addressbook.h \
	configDB.h \
	keyvaluemap.h \
	sections.h

libkab_la_LDFLAGS = -version-info 2:0 $(X_LDFLAGS) $(QT_LDFLAGS)
libkab_la_LIBADD  = -lXext $(LIB_QT)

# just to make sure, automake makes them 
libkab_la_METASOURCES = USE_AUTOMOC cc

DISTCLEANFILES = $(libkab_la_METASOURCES)

picsdir = $(kde_datadir)/kab/pics
pics_DATA = dialog_background.jpg background_1.jpg
