TEMPLATE        = app
CONFIG		= qt dll warn_on thread

unix:DEFINES   = NO_INCLUDE_MOCFILES QT_NO_COMPAT
unix:LIBS       += -lkdevelopqextmdi -L$(KDEDIR)/lib -lkdecore -lkdeui -lDCOP -lkparts
unix:INCLUDEPATH     += .. ../include $(KDEDIR)/include

win32:DEFINES = NO_KDE NO_KDE2
win32:INCLUDEPATH     += ../include $(QTDIR)/include
win32:LIBS      += ../Debug/kmdi.lib $(QTDIR)/lib/qt-mt303.lib $(QTDIR)/lib/qtmain.lib

HEADERS = ./hello.h \
          ./mainwidget.h

SOURCES = ./hello.cpp \
          ./mainwidget.cpp \
          ./main.cpp

TARGET    = kfourchildren
