TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KBOOKMARKS_LIB

win32:LIBS += $$QKWLIB\kdecore$(KDE_VER).lib $$QKWLIB\kdeui$(KDE_VER).lib 
win32:LIBS += $$QKWLIB\kio$(KDE_VER).lib $$QKWLIB\dcop$(KDE_VER).lib


#win32:LIBS += $$QKWLIB\dcop$(KDE_VER).lib $$QKWLIB\kdewin32$(KDE_VER).lib
#win32:LIBS += $$QKWLIB\kssl$(KDE_VER).lib

TARGET		= kbookmarks$$KDEBUG

QMAKE_CXXFLAGS += /I$(QKW)/kdelibs/kio/kfile /I$(QKW)/kdelibs/kio/bookmarks
#/I$(QKW)/kdelibs/kio/misc /I$(QKW)/kdelibs/kio/kssl

system( dcopidl2cpp kbookmarknotifier.kidl )


SOURCES = \
kbookmark.cc \
kbookmarkbar.cc \
kbookmarkdrag.cc \
kbookmarkexporter.cc \
kbookmarkimporter.cc \
kbookmarkmanager.cc \
kbookmarkmenu.cc

# generated:
SOURCES += \
kbookmarknotifier_stub.cpp \
kbookmarknotifier_skel.cpp

HEADERS		= \
kbookmarkimporter.h \
kbookmarkmenu.h \
kbookmarkbar.h \
kbookmarkmanager.h
