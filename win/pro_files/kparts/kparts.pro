TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KPARTS_LIB

TARGET		= kparts$$KDEBUG

win32:LIBS += $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX \
 $$QKWLIB/dcop$$KDELIB_SUFFIX $$QKWLIB/kio$$KDELIB_SUFFIX

QMAKE_CXXFLAGS += /I$(QKW)/kdelibs/kio/kfile

system( bash kmoc )

SOURCES = \
browserextension.cpp \
browserinterface.cpp \
browserrun.cpp \
dockmainwindow.cpp \
event.cpp \
factory.cpp \
historyprovider.cpp \
mainwindow.cpp \
part.cpp \
partmanager.cpp \
plugin.cpp \
statusbarextension.cpp

