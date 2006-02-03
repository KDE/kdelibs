TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KDEUI_LIB

LIBS += $$KDELIBDESTDIR\kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR\kdefx$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX 

system( bash kmoc )
system( bash kdcopidl )

TARGET		= kdeui$$KDEBUG

SOURCES = \
kaboutapplication.cpp \
kaboutdialog.cpp \
kaboutkde.cpp \
kactionclasses.cpp \
kactioncollection.cpp \
kaction.cpp \
kactionselector.cpp \
kactivelabel.cpp \
kanimwidget.cpp \
karrowbutton.cpp \
kauthicon.cpp \
kbugreport.cpp \
kbuttonbox.cpp \
kcharselect.cpp \
kcmenumngr.cpp \
kcmodule.cpp \
kcolorbutton.cpp \
kcolorcombo.cpp \
kcolordialog.cpp \
kcolordrag.cpp \
kcombobox.cpp \
kcommand.cpp \
kcompletionbox.cpp \
kconfigdialog.cpp \
kcursor.cpp \
kdatepicker.cpp \
kdatetbl.cpp \
kdatewidget.cpp \
kdialog.cpp \
kdialogbase.cpp \
kdockwidget.cpp \
kdockwidget_private.cpp \
kdualcolorbutton.cpp \
keditcl1.cpp \
keditcl2.cpp \
keditlistbox.cpp \
kedittoolbar.cpp \
kfontcombo.cpp \
kfontdialog.cpp \
kfontrequester.cpp \
kguiitem.cpp \
khelpmenu.cpp \
kiconview.cpp \
kiconviewsearchline.cpp \
kjanuswidget.cpp \
kkeybutton.cpp \
kkeydialog.cpp \
klanguagebutton.cpp \
kled.cpp \
klineedit.cpp \
klineeditdlg.cpp \
klistbox.cpp \
klistview.cpp \
klistviewsearchline.cpp \
kmainwindowiface.cpp \
kmainwindow.cpp \
kmenubar.cpp \
knuminput.cpp \
knumvalidator.cpp \
kpanelapplet.cpp \
kpanelappmenu.cpp \
kpanelextension.cpp \
kpanelmenu.cpp \
kpassdlg.cpp \
kpassivepopup.cpp \
kpixmapio.cpp \
kpixmapregionselectordialog.cpp \
kpixmapregionselectorwidget.cpp \
kpopupmenu.cpp \
kprogress.cpp \
kpushbutton.cpp \
krestrictedline.cpp \
krootpixmap.cpp \
kruler.cpp \
ksconfig.cpp \
kselect.cpp \
kseparator.cpp \
kshortcutdialog.cpp \
kspell.cpp \
kspelldlg.cpp \
ksplashscreen.cpp \
ksqueezedtextlabel.cpp \
kstatusbar.cpp \
kstdaction.cpp \
kstdguiitem.cpp \
kstringvalidator.cpp \
ksyntaxhighlighter.cpp \
ksystemtray.cpp \
ktabctl.cpp \
ktextbrowser.cpp \
ktextedit.cpp \
ktip.cpp \
ktoolbar.cpp \
ktoolbarbutton.cpp \
ktoolbarhandler.cpp \
ktoolbarradiogroup.cpp \
kurllabel.cpp \
kwhatsthismanager.cpp \
kwindowinfo.cpp \
kwizard.cpp \
kwordwrap.cpp \
kxmlguibuilder.cpp \
kxmlguiclient.cpp \
kxmlguifactory.cpp \
kxmlguifactory_p.cpp \
kdcopactionproxy.cpp \
ktabwidget.cpp \
ktabbar.cpp \
kdatetimewidget.cpp \
ktimewidget.cpp \
kinputdialog.cpp

exists( kmessagebox_win.cpp ) {
 #added KMessageBox::Dangerous implementation
 SOURCES += kmessagebox_win.cpp
}
!exists( kmessagebox_win.cpp ) {
 SOURCES += kmessagebox.cpp
}

# generated:
SOURCES += \
kmainwindowiface_stub.cpp \
kmainwindowiface_skel.cpp

FORMS = \
kshortcutdialog_advanced.ui \
kshortcutdialog_simple.ui \
kspellui.ui

HEADERS =
