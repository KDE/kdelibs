TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KRESOURCES_LIB

win32:INCLUDEPATH	+= $(QKW)/kdelibs/ab

win32:LIBS += $$QKWLIB\kdecore$$KDELIB_SUFFIX $$QKWLIB\kdeui$$KDELIB_SUFFIX \
	$$QKWLIB\dcop$$KDELIB_SUFFIX $$QKWLIB\kio$$KDELIB_SUFFIX 

system( bash kmoc )
system( bash kdcopidl )

TARGET = kresources$$KDEBUG

SOURCES = \
configdialog.cpp \
configpage.cpp \
configwidget.cpp \
factory.cpp \
kcmkresources.cpp \
managerimpl.cpp \
resource.cpp \
selectdialog.cpp \
testresources.cpp

generated:
SOURCES += \
manageriface_skel.cpp \
manageriface_stub.cpp

HEADERS		=
