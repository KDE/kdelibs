
INCLUDES = $(all_includes)

lib_LTLIBRARIES = libThreadWeaver.la

libThreadWeaver_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 4:0
libThreadWeaver_la_LIBADD = $(LIB_QTCORE)

METASOURCES = AUTO

libThreadWeaver_la_SOURCES = \
	DestructedState.cpp \
	WeaverImpl.cpp \
	Job.cpp \
	InConstructionState.cpp \
	State.cpp \
	ShuttingDownState.cpp \
	WeaverInterface.cpp \
	WeaverObserver.cpp \
	SuspendedState.cpp \
	WorkingHardState.cpp \
	DebuggingAids.cpp \
	ThreadWeaver.cpp \
	Thread.cpp \
	SuspendingState.cpp

include ../../admin/Doxyfile.am

