TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KRESOURCES_LIB

INCLUDEPATH	+= $(KDELIBS)/ab

LIBS += $$KDELIBDESTDIR\kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR\kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR\kio$$KDELIB_SUFFIX 

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
