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
CONFIG += qt warn_on thread
TARGET = smiv

test.target=test
test.commands= cd .. && make test
QMAKE_EXTRA_TARGETS += test
