TEMPLATE = app
SOURCES += SMIV.cpp SMIVItem.cpp SMIVModel.cpp SMIVView.cpp \
	SMIVItemDelegate.cpp FileLoaderJob.cpp QImageLoaderJob.cpp \
	ComputeThumbNailJob.cpp
HEADERS += SMIV.h SMIVItem.h SMIVModel.h SMIVView.h \
	SMIVItemDelegate.h FileLoaderJob.h QImageLoaderJob.h \
	ComputeThumbNailJob.h
FORMS += SMIVBase.ui
INCLUDEPATH += ../../Weaver/
LIBS += -L../../Weaver -lThreadWeaver
CONFIG += qt warn_on debug thread
TARGET = smiv
