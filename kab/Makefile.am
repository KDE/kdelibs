
# set the include path for X, qt and KDE
# set "REMOVE_NANA" to get rid of the NANA assertions:
# (REMOVE_NANA is now defined by default if the compiler directives
#  do not define DEBUG), I use MS_NANA_DEFINES for this:
INCLUDES= -I../kdecore -I../kdeui -I../kfile -I../kfmlib $(QT_INCLUDES) $(X_INCLUDES)

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

libkab_la_LDFLAGS = -version-info 2:0
libkab_la_LIBADD  =  -lXext  $(LIB_QT)

# just to make sure, automake makes them 
BUILTSOURCES =	addressbook.moc widgets.moc widget.moc \
		AssertDialog.moc AssertDialogData.moc \
		DialogBase.moc DialogBaseData.moc \
		SearchDialog.moc searchresults.moc \
		editentry.moc datepickerdialog.moc businesscard.moc \
		StringListSelectOneDialog.moc \
		StringListSelectSetDialog.moc \
		StringListSelectAndReorderSet.moc \
		StringListSelectAndReorderSetData.moc \
		SearchDialogMainWidget.moc \
		SearchDialogMainWidgetData.moc


# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

picsdir = $(kde_datadir)/kab/pics
pics_DATA = dialog_background.jpg background_1.jpg

SUFFIXES = .h .moc

.h.moc:
	$(MOC) $< -o $@

widgets.cc:widgets.moc
widget.cc:kab.moc
searchresults.cc:searchresults.moc
editentry.cc:editentry.moc
datepickerdialog.cc:datepickerdialog.moc
businesscard.cc:businesscard.moc
SearchDialog.cpp: SearchDialog.moc 
AddressBookAbout.cpp:AddressBookAbout.moc AddressBookAboutData.moc
PrintDialog.cpp:PrintDialog.moc PrintDialogData.moc
#StringListDialog.cpp:StringListDialog.moc StringListDialogData.moc
StringListEditWidget.cpp:StringListEditWidget.moc StringListEditWidgetData.moc
StringListEditDialog.cpp:StringListEditDialog.moc
kabapi.cc:kabapi.moc
StringListSelectOneDialog.cpp:StringListSelectOneDialog.moc
StringListSelectSetDialog.cpp:StringListSelectSetDialog.moc 
StringListSelectAndReorderSet.cpp:StringListSelectAndReorderSet.moc \
				StringListSelectAndReorderSetData.moc
AssertDialog.cpp: AssertDialog.moc AssertDialogData.moc
DialogBase.cpp			:	DialogBase.moc DialogBaseData.moc
SearchDialogMainWidget.cpp	:	SearchDialogMainWidget.moc \
			   		SearchDialogMainWidgetData.moc


