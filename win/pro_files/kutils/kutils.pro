TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KUTILS_LIB

TARGET		= kutils$$KDEBUG

win32:LIBS += $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX 
win32:LIBS += $$QKWLIB/kio$$KDELIB_SUFFIX 

QMAKE_CXXFLAGS += /I$(QKW)/kdelibs/interfaces/kregexpeditor

system( bash kmoc )

#kcmoduleinfo.cpp \
#kcmoduleloader.cpp \
#kcmoduleproxy.cpp \
#kcmultidialog.cpp \
#kpluginselector.cpp \

SOURCES = \
kfind.cpp \
kfinddialog.cpp \
kmultitabbar.cpp \
kplugininfo.cpp \
kreplace.cpp \
kreplacedialog.cpp

HEADERS		=
