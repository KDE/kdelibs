TEMPLATE	= lib

#QMAKE_EXT_H_MOC = .moc
#QMAKE_H_MOC_MOD = ""
#QMAKE_CPP_MOC_MOD = ""

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KFILE_LIB

win32:LIBS += $$QKWLIB\kdecore$$KDELIB_SUFFIX $$QKWLIB\kdeui$$KDELIB_SUFFIX 
win32:LIBS += $$QKWLIB\kdewin32$$KDELIB_SUFFIX $$QKWLIB\kio$$KDELIB_SUFFIX
win32:LIBS += $$QKWLIB\dcop$$KDELIB_SUFFIX 

TARGET		= kfile$$KDEBUG

QMAKE_CXXFLAGS += /I$(QKW)/kdelibs/kio/kfile /I$(QKW)/kdelibs/kio/bookmarks

system( moc kfiledialog.h -o moc/kfiledialog.moc )
system( moc kencodingfiledialog.h -o moc/kencodingfiledialog.moc )

#system( moc kfiledialog_win_p.h -o moc/kfiledialog_win_p.moc )

SOURCES = \
kcombiview.cpp \
kcustommenueditor.cpp \
kdiroperator.cpp \
kdirselectdialog.cpp \
kdirsize.cpp \
kdiskfreesp.cpp \
kfilebookmarkhandler.cpp \
kfiledetailview.cpp \
kfiledialog_win.cpp \
kfilefiltercombo.cpp \
kfileiconview.cpp \
kfilemetainfowidget.cpp \
kfilepreview.cpp \
kfilemetapreview.cpp \
kfilesharedlg.cpp \
kfilespeedbar.cpp \
kfiletreebranch.cpp \
kfiletreeview.cpp \
kfiletreeviewitem.cpp \
kfileview.cpp \
kicondialog.cpp \
kimagefilepreview.cpp \
kmetaprops.cpp \
knotifydialog.cpp \
knotifywidgetbase.cpp \
kopenwith.cpp \
kpropertiesdialog.cpp \
krecentdirs.cpp \
krecentdocument.cpp \
kurlbar.cpp \
kurlcombobox.cpp \
kurlrequester.cpp \
kurlrequesterdlg.cpp \
kencodingfiledialog.cpp

# from kbookmarks:
system( cd .. && cd bookmarks && dcopidl2cpp kbookmarknotifier.kidl )
system( cd .. && cd bookmarks && dcopidl2cpp kbookmarkmanager.kidl )

SOURCES += \
../bookmarks/kbookmark.cc \
../bookmarks/kbookmarkbar.cc \
../bookmarks/kbookmarkdrag.cc \
../bookmarks/kbookmarkexporter.cc \
../bookmarks/kbookmarkimporter.cc \
../bookmarks/kbookmarkmanager.cc \
../bookmarks/kbookmarkmenu.cc \
../bookmarks/kbookmarknotifier_stub.cpp \
../bookmarks/kbookmarknotifier_skel.cpp \
../bookmarks/kbookmarkmanager_stub.cpp \
../bookmarks/kbookmarkmanager_skel.cpp \
../../kdecore/kprotocolinfo_kdecore.cpp

# js 2004-08-05 ^^^^^ a hack because msvc cannot split a class between two libraries!

HEADERS += \
../bookmarks/kbookmarkimporter.h \
../bookmarks/kbookmarkmenu.h \
../bookmarks/kbookmarkbar.h \
../bookmarks/kbookmarkmanager.h

SOURCES += \
../kio/kdirnotify_stub.cpp
