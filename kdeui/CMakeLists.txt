
project(kdeui)

include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${CMAKE_SOURCE_DIR}/interfaces )
include_directories( ${KDE4_KDECORE_INCLUDES} )
add_definitions(${QDBUS_DEFINITIONS})

remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

if (UNIX AND NOT APPLE)
   add_subdirectory( kdetrayproxy )
endif (UNIX AND NOT APPLE)

add_subdirectory( tests )
add_subdirectory( about )


########### next target ###############

set(kdeui_LIB_SRCS
 ksharedpixmap.cpp
 kmenu.cpp
 ktoolbar.cpp
 kaction.cpp
 kstdaction.cpp
 kactioncollection.cpp
 ktoggleaction.cpp
 kselectaction.cpp
 krecentfilesaction.cpp
 kfontaction.cpp
 kfontsizeaction.cpp
 kactionmenu.cpp
 ktoolbarpopupaction.cpp
 ktoggletoolbaraction.cpp
 ktogglefullscreenaction.cpp
 k3widgetaction.cpp
 kpastetextaction.cpp
 kseparatoraction.cpp
 kurllabel.cpp
 kmenubar.cpp
 kinputdialog.cpp
 knuminput.cpp
 klineedit.cpp
 k3listview.cpp
 knotificationrestrictions.cpp
 kprogressdialog.cpp
 kprogressbar.cpp
 kcolordialog.cpp
 kselector.cpp
 kdatepicker.cpp
 kdatetable.cpp
 kfontrequester.cpp
 kfontdialog.cpp
 ktabctl.cpp
 kstatusbar.cpp
 kmainwindow.cpp
 keditlistbox.cpp
 kseparator.cpp
 krestrictedline.cpp
 kcolorbutton.cpp
 ksystemtray.cpp
 kbuttonbox.cpp
 keditcl1.cpp
 keditcl2.cpp
 kled.cpp
 kshortcutdialog.cpp
 kkeybutton.cpp
 kkeydialog.cpp
 kruler.cpp
 kcursor.cpp
 kcharselect.cpp
 kcolormimedata.cpp
 knumvalidator.cpp
 kdialog.cpp
 kdialogbase.cpp
 kjanuswidget.cpp
 kaboutdialog.cpp
 kauthicon.cpp
 kmessagebox.cpp
 kdualcolorbutton.cpp
 ktextbrowser.cpp
 khelpmenu.cpp
 k3iconview.cpp
 klistbox.cpp
 kwordwrap.cpp
 kbugreport.cpp
 kcombobox.cpp
 kpassworddialog.cpp
 kxmlguiclient.cpp
 kxmlguifactory.cpp
 kxmlguifactory_p.cpp
 kxmlguibuilder.cpp
 kmenumenuhandler_p.cpp
 kedittoolbar.cpp
 kanimwidget.cpp
 krootpixmap.cpp
 kaboutkde.cpp
 kaboutapplication.cpp
 kcolorcombo.cpp
 kpushbutton.cpp
 kcompletionbox.cpp
 ksqueezedtextlabel.cpp
 kcmodule.cpp
 kcommand.cpp
 kwindowlistmenu.cpp
 kfontcombo.cpp
 ktip.cpp
 kdatewidget.cpp
 karrowbutton.cpp
 kmainwindowiface.cpp
 kguiitem.cpp
 kstdguiitem.cpp
 kstringvalidator.cpp
 kactivelabel.cpp
 ktextedit.cpp
 kpassivepopup.cpp
 ktoolbarhandler.cpp
 kdatetimewidget.cpp
 ktimezonewidget.cpp
 ksconfig.cpp
 kspelldlg.cpp
 kspell.cpp
 ktabbar.cpp
 ktabwidget.cpp
 ksplashscreen.cpp
 kconfigdialog.cpp
 kactionselector.cpp
 klanguagebutton.cpp
 k3listviewsearchline.cpp
 kpixmapregionselectorwidget.cpp
 kpixmapregionselectordialog.cpp
 kwhatsthismanager.cpp
 k3iconviewsearchline.cpp
 ktoolbarlabelaction.cpp
 khbox.cpp
 kvbox.cpp
 kconfigdialogmanager.cpp
 knotification.cpp
 knotificationmanager.cpp
 kiconengine.cpp
 kicon.cpp
 ktoolbarspaceraction.cpp
 kmanagerselection.cpp
 kglobalaccel.cpp
 ktreewidgetsearchline.cpp
 kkeyserver.cpp
 kcompletionbase.cpp
 kcompletion.cpp
)

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} kglobalaccel_x11.cpp kkeyserver_x11.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} kglobalaccel_mac.cpp kkeyserver_mac.cpp )
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} kglobalaccel_win.cpp )
endif (Q_WS_WIN)


kde4_automoc(${kdeui_LIB_SRCS})

set( kdeui_UI
   kspellui.ui
   kkeydialog.ui
   kshortcutdialog_simple.ui
   kshortcutdialog_advanced.ui
   )

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(kdeui PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install_targets(${LIB_INSTALL_DIR} kdeui )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeui/pics FILES aboutkde.png ktip-bulb.png ktip-background.png )
install_files( ${CONFIG_INSTALL_DIR}/ui FILES ui_standards.rc )
install_files( /include FILES
 kprogressbar.h
 kprogressdialog.h
 kcolordialog.h
 kselector.h
 kdatepicker.h
 kdatetable.h
 kfontdialog.h
 kmenu.h
 kfontrequester.h
 ktabctl.h
 kstatusbar.h
 kmainwindow.h
 ktoolbar.h
 kmenubar.h
 knuminput.h
 knotificationrestrictions.h
 kseparator.h
 klineedit.h
 krestrictedline.h
 kcolorbutton.h
 ksystemtray.h
 kbuttonbox.h
 keditcl.h
 kled.h
 keditlistbox.h
 kkeydialog.h
 kkeybutton.h
 kurllabel.h
 kruler.h
 kcursor.h
 kinputdialog.h
 kactivelabel.h
 kcharselect.h
 kcolormimedata.h
 knumvalidator.h
 kdialog.h
 kdialogbase.h
 kjanuswidget.h
 kaboutdialog.h
 kauthicon.h
 kmessagebox.h
 ksharedpixmap.h
 kdualcolorbutton.h
 ktextbrowser.h
 kaction.h
 kactioncollection.h
 kactionclasses.h
 kseparatoraction.h
 ktoggleaction.h
 kselectaction.h
 krecentfilesaction.h
 kfontaction.h
 kfontsizeaction.h
 kactionmenu.h
 ktoolbarpopupaction.h
 ktoggletoolbaraction.h
 ktogglefullscreenaction.h
 k3widgetaction.h
 kpastetextaction.h
 khelpmenu.h
 kcmodule.h
 kstdaction.h
 kcombobox.h
 k3iconview.h
 k3listview.h
 klistbox.h
 kbugreport.h
 kpassworddialog.h
 kxmlguiclient.h
 kxmlguibuilder.h
 kxmlguifactory.h
 kwordwrap.h
 kedittoolbar.h
 kanimwidget.h
 krootpixmap.h
 kaboutkde.h
 kaboutapplication.h
 kpushbutton.h
 kcolorcombo.h
 kcompletionbox.h
 ksqueezedtextlabel.h
 kcommand.h
 kwindowlistmenu.h
 kfontcombo.h
 kaccelgen.h
 ktip.h
 kdatewidget.h
 karrowbutton.h
 kguiitem.h
 kstdguiitem.h
 kstringvalidator.h
 ktextedit.h
 kpassivepopup.h
 kdatetimewidget.h
 ktimezonewidget.h
 kspell.h
 ksconfig.h
 kspelldlg.h
 ktabbar.h
 ktabwidget.h
 ksplashscreen.h
 kconfigdialog.h
 kactionselector.h
 klanguagebutton.h
 k3listviewsearchline.h
 kpixmapregionselectorwidget.h
 kpixmapregionselectordialog.h
 k3iconviewsearchline.h
 ktoolbarlabelaction.h
 kshortcutdialog.h
 khbox.h
 kvbox.h
 kconfigdialogmanager.h
 knotification.h
 kiconengine.h
 kicon.h
 ktoolbarspaceraction.h
 kmanagerselection.h
 kglobalaccel.h
 ktreewidgetsearchline.h
 kkeyserver_x11.h
 kkeyserver.h
 kcompletion.h
)

install(PROGRAMS preparetips DESTINATION bin)




#original Makefile.am contents follow:

#
##	This file is part of the KDE libraries
##    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
##			  (C) 1997 Stephan Kulow (coolo@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU General Public License
##    along with this library; see the file COPYING.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#SUBDIRS = . kdetrayproxy tests about
#
#INCLUDES= -I$(top_srcdir)/kdefx -I$(top_srcdir)/interfaces $(all_includes)
#
## For the future: examine if condensing the tons of *_LDFLAGS variables
## into $(all_libraries) isn't better
#AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)
#
#lib_LTLIBRARIES = libkdeui.la libkspell.la
#libkdeui_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined -version-info 6:0:2
#libkdeui_la_LIBADD = ../kdecore/libkdecore.la ../kdefx/libkdefx.la
#
#picsdir = $(kde_datadir)/kdeui/pics
#pics_DATA = aboutkde.png ktip-bulb.png ktip-background.png
#
#include_HEADERS = kprogress.h kcolordialog.h kselect.h \
#	kdatepicker.h kdatetbl.h kdatetable.h kfontdialog.h kmenu.h kfontrequester.h ktabctl.h \
#	kstatusbar.h kmainwindow.h kmainwindowiface.h ktoolbar.h kmenubar.h knuminput.h \
#	kseparator.h klineedit.h krestrictedline.h kcolorbutton.h ksystemtray.h \
#	kbuttonbox.h keditcl.h kled.h keditlistbox.h kwizard.h kkeydialog.h kkeybutton.h \
#	kurllabel.h kruler.h kcursor.h kinputdialog.h kactivelabel.h kcharselect.h \
#	kcolormimedata.h knumvalidator.h kdialog.h kdialogbase.h kjanuswidget.h kaboutdialog.h \
#	kauthicon.h kmessagebox.h ksharedpixmap.h kdualcolorbutton.h \
#	ktoolbarbutton.h ktoolbarradiogroup.h ktextbrowser.h kaction.h kactioncollection.h \
#	kactionclasses.h khelpmenu.h kcmodule.h kcmenumngr.h kactionshortcutlist.h kstdaction.h \
#	kcombobox.h k3iconview.h k3listview.h klistbox.h kbugreport.h kpassdlg.h \
#	kxmlguiclient.h kxmlguibuilder.h kxmlguifactory.h kwordwrap.h \
#	kedittoolbar.h kanimwidget.h krootpixmap.h kaboutkde.h kaboutapplication.h \
#	kpushbutton.h kdcopactionproxy.h kcolorcombo.h kcompletionbox.h ksqueezedtextlabel.h \
#	kcommand.h kwindowlistmenu.h kfontcombo.h kaccelgen.h ktip.h kdatewidget.h \
#	karrowbutton.h kguiitem.h kstdguiitem.h kstringvalidator.h kwindowinfo.h ktextedit.h \
#	kpassivepopup.h kdatetimewidget.h ktimezonewidget.h kscrollview.h kspell.h ksconfig.h \
#	kspelldlg.h ksyntaxhighlighter.h ktabbar.h ktabwidget.h ksplashscreen.h \
#	kconfigdialog.h kactionselector.h klanguagebutton.h k3listviewsearchline.h \
#	kpixmapregionselectorwidget.h kpixmapregionselectordialog.h k3iconviewsearchline.h \
#	ktoolbarlabelaction.h kshortcutdialog.h khbox.h kvbox.h \
#	kconfigdialogmanager.h knotification.h
#
## the order of the sources isn't randomly (at least not completely).
## the order is predictated by the use of X11 header files
## the files on the first line need to occur first when compiling
## with --enable-final since they use X defines that get undefined
## in ksharedpixmap.cpp
#libkdeui_la_SOURCES = ksharedpixmap.cpp kmenu.cpp ktoolbar.cpp \
#	kaction.cpp kstdaction.cpp kactioncollection.cpp kactionclasses.cpp kurllabel.cpp \
#	kmenubar.cpp kinputdialog.cpp knuminput.cpp klineedit.cpp k3listview.cpp \
#	kprogress.cpp kcolordialog.cpp kselect.cpp kdatepicker.cpp kdatetbl.cpp \
#	kfontrequester.cpp kfontdialog.cpp ktabctl.cpp kstatusbar.cpp kmainwindow.cpp \
#	keditlistbox.cpp kscrollview.cpp kseparator.cpp krestrictedline.cpp kcolorbutton.cpp \
#	ksystemtray.cpp kbuttonbox.cpp keditcl1.cpp keditcl2.cpp kled.cpp \
#	kshortcutdialog_simple.ui3 kshortcutdialog_advanced.ui3 kshortcutdialog.cpp kkeybutton.cpp \
#	kkeydialog.cpp kruler.cpp kcursor.cpp kcharselect.cpp kcolormimedata.cpp knumvalidator.cpp \
#	kdialog.cpp kdialogbase.cpp kjanuswidget.cpp kaboutdialog.cpp kauthicon.cpp \
#	kmessagebox.cpp kdualcolorbutton.cpp ktoolbarradiogroup.cpp ktoolbarbutton.cpp \
#	ktextbrowser.cpp khelpmenu.cpp kcmenumngr.cpp k3iconview.cpp klistbox.cpp kwordwrap.cpp \
#	kbugreport.cpp kcombobox.cpp kpassdlg.cpp kxmlguiclient.cpp kxmlguifactory.cpp \
#	kxmlguifactory_p.cpp kxmlguibuilder.cpp kedittoolbar.cpp kanimwidget.cpp krootpixmap.cpp \
#	kaboutkde.cpp kaboutapplication.cpp kdcopactionproxy.cpp kcolorcombo.cpp \
#	kpushbutton.cpp kcompletionbox.cpp ksqueezedtextlabel.cpp kcmodule.cpp kcommand.cpp \
#	kwindowlistmenu.cpp kfontcombo.cpp ktip.cpp kdatewidget.cpp karrowbutton.cpp \
#	kmainwindowiface.cpp kmainwindowiface.skel kguiitem.cpp kstdguiitem.cpp kstringvalidator.cpp \
#	kactivelabel.cpp kwindowinfo.cpp ktextedit.cpp kwizard.cpp kpassivepopup.cpp \
#	ktoolbarhandler.cpp kdatetimewidget.cpp ktimezonewidget.cpp ksconfig.cpp kspelldlg.cpp \
#	kspell.cpp ksyntaxhighlighter.cpp ktabbar.cpp ktabwidget.cpp ksplashscreen.cpp \
#	kspellui.ui kconfigdialog.cpp kactionselector.cpp klanguagebutton.cpp \
#	k3listviewsearchline.cpp kpixmapregionselectorwidget.cpp kpixmapregionselectordialog.cpp \
#	kwhatsthismanager.cpp k3iconviewsearchline.cpp ktoolbarlabelaction.cpp khbox.cpp kvbox.cpp \
#	kconfigdialogmanager.cpp knotification.cpp knotificationmanager.cpp knotificationmanager.skel
#
#noinst_HEADERS = k3listviewlineedit.h \
#		kdialogbase_priv.h kaboutdialog_private.h kcursor_private.h \
#		kdepackages.h kxmlguifactory_p.h \
#		kstdaction_p.h kwhatsthismanager_p.h knotificationmanager.h
#
#scripts_SCRIPTS = preparetips
#scriptsdir = $(kde_bindir)
#
#libkdeui_la_METASOURCES = AUTO
#libkdeui_la_NMCHECK = $(srcdir)/libkdeui.nmcheck
#libkdeui_la_NMCHECKWEAK = $(srcdir)/libkdeui_weak.nmcheck $(top_srcdir)/kdecore/libkdecore_weak.nmcheck \
#	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/kdecore/libqt-mt_weak.nmcheck
#
#rcdir = $(kde_confdir)/ui
#rc_DATA = ui_standards.rc
#
#EXTRA_DIST = CHANGES.kdatepicker USERS.kdatepicker dcolorreset.xpm kpartgui.dtd dcolorarrow.xbm colors Mainpage.dox
#
#.PHONY: kdepackages
#
#kdepackages:
#	-rm $(srcdir)/kdepackages.h
#	curl "http://bugs.kde.org/xml.cgi?data=versiontable" > tmp_download
#	echo "// DO NOT EDIT - EDIT bugs/Maintainers instead" >  $(srcdir)/kdepackages.h
#	echo "const char * const packages[] = {" >> $(srcdir)/kdepackages.h
#	cat tmp_download | egrep "product|component" | grep -v "</product>" | \
#	sed -e "s,<product name=\"\([^\"]*\)\">,product=\"\1\"; component=\"general\",; \
#	s#<component>\([^<]*\)</component>#component=\"\1\"#"  > tmp_download2
#	(while read i; do eval $$i; echo "\"$$product/$$component\","; done) < tmp_download2 | \
#	sed -e "s,/general\",/00general\"," | LC_ALL=C sort -u | \
#	grep -v 'valgrind/' | sed -e "s,/00general\",\"," >> $(srcdir)/kdepackages.h
#	echo "0 };" >>  $(srcdir)/kdepackages.h
#	rm tmp_download2 tmp_download
#
#DOXYGEN_REFERENCES = kdecore kdefx dcop kio kutils
#DOXYGEN_SET_INTERNAL_DOCS = YES
#include ../admin/Doxyfile.am
