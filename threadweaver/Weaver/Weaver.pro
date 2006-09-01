TEMPLATE = lib

SOURCES += WeaverInterface.cpp \
	ThreadWeaver.cpp WeaverImpl.cpp \
	DebuggingAids.cpp Thread.cpp Job.cpp \
	State.cpp \
	StateImplementation.cpp \
	InConstructionState.cpp WorkingHardState.cpp \
	SuspendingState.cpp SuspendedState.cpp \
	ShuttingDownState.cpp DestructedState.cpp \
        WeaverObserver.cpp \
	JobCollection.cpp \
	JobSequence.cpp \
	DependencyPolicy.cpp \
        ResourceRestrictionPolicy.cpp 

HEADERS += WeaverInterface.h \
	ThreadWeaver.h WeaverImpl.h \
	DebuggingAids.h Thread.h Job.h \
	State.h \
	StateImplementation.h \
	InConstructionState.h WorkingHardState.h \
	SuspendingState.h SuspendedState.h \
        ShuttingDownState.h DestructedState.h \
        WeaverObserver.h \
	JobCollection.h \
	JobSequence.h \
	QueuePolicy.h \
	DependencyPolicy.h \
        ResourceRestrictionPolicy.h

CONFIG += dll warn_on thread
QT -= gui
VERSION = 0.6.0
TARGET = ThreadWeaver

test.target=test
test.commands= cd .. && $(MAKE) test
QMAKE_EXTRA_TARGETS += test
