# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# just set the variable
APPSDIR = $(kde_appsdir)/Utilities
# set the include path for X, qt and KDE
# set "REMOVE_NANA" to get rid of the NANA assertions:
# (REMOVE_NANA is now defined by default if the compiler directives
#  do not define DEBUG), I use MS_NANA_DEFINES for this:
INCLUDES= -I../kdecore -I../kdeui -I../kfile -I../kfmlib @all_includes@ 
# $(MS_NANA_DEFINES)
# -DREMOVE_NANA
# claim, which subdirectories you want to install
# SUBDIRS =

####### This part is very khexdit specific
# you can add here more. This one gets installed 
lib_LTLIBRARIES = libkab.la
KABVERSION=	0.0.1
KABPATCH=	0
# Which sources should be compiled for khexdit.
libkab_la_SOURCES	= \
	kabapi.cc \
	PrintDialog.cpp \
	PrintDialogData.cpp \
	PageSetupWidget.cpp \
	PageSetupWidgetData.cpp \
	SearchDialog.cpp \
	SearchDialogData.cpp \
	StringListDialog.cpp \
	StringListDialogData.cpp \
	addressbook.cc \
	businesscard.cc \
	datepickerdialog.cc \
	editentry.cc \
	jpeg.cpp \
	searchresults.cc \
	widget.cc \
	configDB.cc \
	sections.cc \
	keyvaluemap.cc \
	functions.cc \
	StringListSelectOneDialog.cpp \
	StringListSelectOneDialogData.cpp \
	StringListSelectSetDialog.cpp \
	StringListSelectSetDialogData.cpp \
	StringListSelectAndReorderSet.cpp \
	StringListSelectAndReorderSetData.cpp \
	widgets.cc 

include_HEADERS = \
	stl_headers.h \
	kabapi.h \
	kab.h \
	addressbook.h \
	configDB.h \
	keyvaluemap.h \
	sections.h
#	PrintDialog.h \
#	PrintDialogData.h \
#	SearchDialog.h \
#	SearchDialogData.h \
#	StringListDialog.h \
#	StringListDialogData.h \
#	TalkDialog.h \
#	TalkDialogData.h \
#	businesscard.h \
#	datepickerdialog.h \
#	debug.h \
#	editentry.h \
#	jpeg.h \
#	jpeginc.h \
#	searchresults.h \
#	widgets.h

# the library search path
libkab_la_LDFLAGS = @all_libraries@ 

# Usually I define MS_NANA_LIBS to "-lnana" if DEBUG is defined as a 
# compiler directive.
libkab_la_LIBADD   = -lkfm -lkdeui -lkdecore -lqt \
	-lX11 -lXext @LIBSOCKET@ 
# $(MS_NANA_LIBS)

# this option you can leave out. Just, if you use "make dist", you need it
# noinst_HEADERS = 
# just to make sure, automake makes them 
BUILTSOURCES =	addressbook.moc widgets.moc widget.moc \
		SearchDialog.moc searchresults.moc \
		editentry.moc datepickerdialog.moc businesscard.moc \
		StringListDialog.moc StringListDialogData.moc \
		StringListSelectOneDialog.moc \
		StringListSelectOneDialogData.moc \
		StringListSelectSetDialog.moc \
		StringListSelectSetDialogData.moc \
		StringListSelectAndReorderSet.moc \
		StringListSelectAndReorderSetData.moc

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kab_SOURCES)

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local:

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:

# the MOC files:

%.moc:%.h
	$(MOC) $< -o $@



# addressbook.cc:addressbook.moc
# addressbook.moc:addressbook.h

widgets.cc:widgets.moc
widgets.moc:widgets.h

widget.cc:widget.moc
widget.moc:kab.h
	$(MOC) $< -o $@

searchresults.cc:searchresults.moc
searchresults.moc:searchresults.h

editentry.cc:editentry.moc
editentry.moc:editentry.h

datepickerdialog.cc:datepickerdialog.moc
datepickerdialog.moc:datepickerdialog.h

businesscard.cc:businesscard.moc
businesscard.moc:businesscard.h

SearchDialog.cpp:SearchDialog.moc SearchDialogData.moc
SearchDialog.moc:SearchDialog.h
SearchDialogData.moc:SearchDialogData.h

AddressBookAbout.cpp:AddressBookAbout.moc AddressBookAboutData.moc
AddressBookAbout.moc:AddressBookAbout.h
AddressBookAboutData.moc:AddressBookAboutData.h

PrintDialog.cpp:PrintDialog.moc PrintDialogData.moc
PrintDialog.moc:PrintDialog.h
PrintDialogData.moc:PrintDialogData.h

PageSetupWidget.cpp:PageSetupWidget.moc PageSetupWidgetData.moc
PageSetupWidget.moc:PageSetupWidget.h
PageSetupWidgetData.moc:PageSetupWidgetData.h

StringListDialog.cpp:StringListDialog.moc StringListDialogData.moc
StringListDialog.moc:StringListDialog.h
StringListDialogData.moc:StringListDialogData.h

kabapi.cc:kabapi.moc
kabapi.moc:kabapi.h

StringListSelectOneDialog.cpp:StringListSelectOneDialog.moc \
				StringListSelectOneDialogData.moc
StringListSelectOneDialog.moc:StringListSelectOneDialog.h
StringListSelectOneDialogData.moc:StringListSelectOneDialogData.h

StringListSelectSetDialog.cpp:StringListSelectSetDialog.moc \
				StringListSelectSetDialogData.moc
StringListSelectSetDialog.moc:StringListSelectSetDialog.h
StringListSelectSetDialogData.moc:StringListSelectSetDialogData.h

StringListSelectAndReorderSet.cpp:StringListSelectAndReorderSet.moc \
				StringListSelectAndReorderSetData.moc
StringListSelectAndReorderSet.moc:StringListSelectAndReorderSet.h
StringListSelectAndReorderSetData.moc:StringListSelectAndReorderSetData.h
