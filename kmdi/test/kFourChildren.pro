TEMPLATE        = app
CONFIG		= qt warn_on thread

unix:DEFINES   = NO_INCLUDE_MOCFILES QT_NO_COMPAT

unix:LIBS       += -lkmdi -L$(KDEDIR)/lib -lkdecore -lkdeui -lDCOP -lkparts

INCLUDEPATH     += $(KDEDIR)/include ..

HEADERS = ./hello.h \
          ./mainwidget.h

SOURCES = ./hello.cpp \
          ./mainwidget.cpp \
          ./main.cpp

TARGET    = kfourchildren
