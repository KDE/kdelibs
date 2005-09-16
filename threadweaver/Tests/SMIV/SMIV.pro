TEMPLATE = app
SOURCES += SMIV.cpp SMIVItem.cpp SMIVModel.cpp SMIVView.cpp \
	SMIVItemDelegate.cpp QImageLoaderJob.cpp \
	ComputeThumbNailJob.cpp
HEADERS += SMIV.h SMIVItem.h SMIVModel.h SMIVView.h \
	SMIVItemDelegate.h QImageLoaderJob.h \
	ComputeThumbNailJob.h
FORMS += SMIVBase.ui
INCLUDEPATH += ../../Weaver/ ../../Experimental
LIBS += -L../../Weaver -lThreadWeaver -L../../Experimental -lThreadWeaverExperimental
CONFIG += qt warn_on debug thread
TARGET = smiv
