TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KMDI_LIB

TARGET		= kmdi$$KDEBUG

win32:LIBS += $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX 
win32:LIBS += $$QKWLIB/kutils$$KDELIB_SUFFIX $$QKWLIB/kparts$$KDELIB_SUFFIX

QMAKE_CXXFLAGS += /I res /I$(QKW)/kdelibs/kutils /I$(QKW)/kdeui

system( bash kmoc )

SOURCES = \
kmdichildarea.cpp \
kmdichildfrm.cpp \
kmdichildfrmcaption.cpp \
kmdichildview.cpp \
kmdidockcontainer.cpp \
kmdidocumentviewtabwidget.cpp \
kmdifocuslist.cpp \
kmdiguiclient.cpp \
kmdimainfrm.cpp \
kmditaskbar.cpp \
kmditoolviewaccessor.cpp

HEADERS		=
