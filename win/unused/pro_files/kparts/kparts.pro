TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KPARTS_LIB

TARGET		= kparts$$KDEBUG

LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX \
 $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX

INCLUDEPATH += $(KDELIBS)/kio/kfile

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

