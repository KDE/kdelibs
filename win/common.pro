#
# common.pro - Common definitions for KDElibs/win32 .pro files
# (c) 2003-2005, Jaroslaw Staniek, js@iidea.pl
#

!win32{
	error(This is a windows only lib!)
}

# to avoid a need for using Q_WS_WIN in C source code
DEFINES += _WINDOWS WIN32_LEAN_AND_MEAN

INCLUDEPATH += .. ../include
win32-g++: {
	INCLUDEPATH += ../include/mingw
}

win32-msvc*: {
	INCLUDEPATH += ../include/msvc
}

DESTDIR = ../lib
DLLDESTDIR = ../bin
#INSTALLS += target 

# custom definitions, options on which KDElibs do not depend
exists( custom_defs.pro ) {
  include( custom_defs.pro )
}

CONFIG += qt thread warn_on build_all create_prl

!debug_and_release|build_pass {
   CONFIG(debug, debug|release) {
      TARGET = $$member(TARGET, 0)d
   }
}


