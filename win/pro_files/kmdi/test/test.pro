TEMPLATE        = app
#CONFIG		= qt warn_on thread

win32:include( $(QKW)/kwcommon.pro )

CONFIG -= release
CONFIG -= windows
CONFIG += debug
CONFIG += console

unix:DEFINES   = NO_INCLUDE_MOCFILES QT_NO_COMPAT
unix:LIBS       += -lkmdi -L$(KDEDIR)/lib -lkdecore -lkdeui -lDCOP -lkparts
unix:INCLUDEPATH     += . .. $(KDEDIR)/include

win32:LIBS += $$QKWLIB\kmdi$$KDELIB_SUFFIX $$QKWLIB\kparts$$KDELIB_SUFFIX
#$$QKWLIB\kexidb$$KEXILIB_SUFFIX $$QKWLIB\kexifilter$$KEXILIB_SUFFIX \
# $$QKWLIB\kfile$$KDELIB_SUFFIX  \
# $$QKWLIB\kio$$KDELIB_SUFFIX $$QKWLIB\kstore$$KDELIB_SUFFIX $$QKWLIB\kdewin32$$KDELIB_SUFFIX

#win32:DEFINES = NO_KDE NO_KDE2
#win32:INCLUDEPATH     += . .. $(QTDIR)/include
#win32:LIBS      += ../Debug/kmdi.lib $(QTDIR)/lib/qt-mt303.lib $(QTDIR)/lib/qtmain.lib

HEADERS = hello.h \
          mainwidget.h

SOURCES = hello.cpp \
          mainwidget.cpp \
          main.cpp

TARGET    = kfourchildren
