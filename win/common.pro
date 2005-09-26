#
# common.pro - Common definitions for KDElibs/win32 .pro files
# (c) 2003-2005, Jaroslaw Staniek, js@iidea.pl
#

# to avoid a need for using Q_WS_WIN in C source code
DEFINES += _WINDOWS WIN32_LEAN_AND_MEAN

# custom definitions, options on which KDElibs do not depend
exists( custom_defs.pro ) {
  include( custom_defs.pro )
}

# common version info for all libs:
!contains( CONFIG, kde3lib ) {
	VER_MAJ = $(KDE_VER_MAJ)
	VER_MIN = $(KDE_VER_MIN)
	VER_PAT = $(KDE_VER_PAT)
}

CONFIG += qt thread warn_on 
isEmpty( KW_CONFIG ) {
  KW_CONFIG += debug
  #KW_CONFIG += release
  #KW_CONFIG += windows
  KW_CONFIG += console
}

# release switch has priority over debug
contains(KW_CONFIG,release) {
  CONFIG -= debug
  CONFIG += release
}
contains(KW_CONFIG,debug) {
  CONFIG += debug
  CONFIG -= release
}
contains(KW_CONFIG,windows) {
  CONFIG += windows
  CONFIG -= console
}
contains(KW_CONFIG,console) {
  CONFIG -= windows
  CONFIG += console
}

# global binary destination directory
isEmpty( KDEBINDESTDIR ) {
	KDEBINDESTDIR = $(KDEDIR)
}

# global library destination directory
KDELIBDESTDIR = $$KDEBINDESTDIR\lib

# dlls suffixes for given target
isEmpty( KDEBUG ) {
  contains(CONFIG,debug) {
		KDEBUG=_d
		KDELIBDEBUG=_d
		KDELIBDEBUGLIB=_d.lib
  }
  !contains(CONFIG,debug) {
		KDEBUG=_
		contains(CONFIG,kde3lib) {
			KDELIBDEBUG=
		}
		!contains(CONFIG,kde3lib) {
			KDELIBDEBUG=_
		}
		KDELIBDEBUGLIB=.lib
  }
}
KDELIB_SUFFIX=$$KDEBUG$(KDE_VER).lib

contains( TEMPLATE, app ) {
  # default dest dir for "app"
#  contains(CONFIG,debug) {
    DESTDIR = $$KDEBINDESTDIR\bin
#  }
#  !contains(CONFIG,debug) {
#    DESTDIR = $$KDEBINDESTDIR\release-bin
#  }
	!contains(CONFIG,nokdecore) {
			LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX
	}
	!contains(CONFIG,nokdeui) {
			LIBS += $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX
	}
	!contains(CONFIG,nokdefx) {
			LIBS += $$KDELIBDESTDIR/kdefx$$KDELIB_SUFFIX
	}
}
# default template is "lib"
isEmpty( TEMPLATE ) {
  TEMPLATE = lib
}
contains( TEMPLATE, lib ) {
	CONFIG	+= dll

	# indicate that we building a library
	QMAKE_CXXFLAGS += -DKDE_MAKE_LIB=1

	# lib/kde3 dest dir (for modules)
	contains( CONFIG, kde3lib ) {
	  DESTDIR		= $$KDELIBDESTDIR/kde3
		TARGET_EXT = .dll #- no ver. in filename
	}
	!contains( CONFIG, kde3lib ) {
	  DESTDIR		= $$KDELIBDESTDIR
	}
	contains( CONFIG, kstyle ) {
	  DESTDIR		= $$KDELIBDESTDIR/kde3/plugins/styles
		CONFIG += plugin
	}
#  !contains(CONFIG,debug) {
#    DESTDIR = $$KDEBINDESTDIR\release-lib
#  }

  VERSION		= $(KDE_VER_MAJ).$(KDE_VER_MIN).$(KDE_VER_PAT)
#  VER_MAJ	= $(KDE_VER_MAJ)
#  VER_MIN = $(KDE_VER_MIN)
#  VER_PAT = $(KDE_VER_PAT)
}

# win32 dependent lib
!contains( DEFINES, MAKE_KDEWIN32_LIB ) {
  LIBS += $$KDELIBDESTDIR/kdewin32$$KDELIB_SUFFIX
}

# libltdl:
!contains( DEFINES, MAKE_LTDL_LIB ) {
  !contains( DEFINES, MAKE_KDEWIN32_LIB ) {
    LIBS += $$KDELIBDESTDIR/ltdl$$KDELIB_SUFFIX
    DEFINES += LIBLTDL_DLL_IMPORT
  }
}

!contains( DEFINES, QT_DLL) {
  DEFINES += QT_DLL
}

# global definitions
win32-borland {
    QMAKE_CXXFLAGS += /I $(KDELIBS)/win/kdelibs_global_win.h
    QMAKE_CFLAGS += /I $(KDELIBS)/win/kdelibs_global_win.h
}
win32-msvc* {
    QMAKE_CXXFLAGS += /FI$(KDELIBS)/win/kdelibs_global_win.h
    QMAKE_CFLAGS += /FI$(KDELIBS)/win/kdelibs_global_win.h
}

# enable Run-Time Type Information (needed by dynamic_cast)
QMAKE_CXXFLAGS += QMAKE_CFLAGS_RTTI_ON  # /GR for msvc

# enables synchronous exception 
QMAKE_CXXFLAGS += QMAKE_CFLAGS_EXCEPTIONS_ON # /GX for msvc

# Language Extensions)
QMAKE_CXXFLAGS += /Ze

# catch Release-Build Errors in Debug Build
#DISABLED for msvc.net
#contains( CONFIG, debug ) {
#	contains( KW_CONFIG, debug ) {
#		QMAKE_CXXFLAGS += /GZ
#	}
#}

# create an output file whether or not LINK finds an undefined symbol
# (warning 4006 will be raised instead of error):
QMAKE_LFLAGS += /FORCE:MULTIPLE

# Specify that filename is a C++ source file, even if it doesn’t have 
# a .cpp or .cxx extension, thus .cc files are compiled properly with msvc
QMAKE_CXXFLAGS += /TP

INCLUDEPATH	+= moc $(KDELIBS)/win $(KDELIBS)

contains(KW_CONFIG,release) {
OBJECTS_DIR = obj_rel
}
!contains(KW_CONFIG,release) {
OBJECTS_DIR = obj
}

MOC_DIR = moc

# enable this to temporary add debug info!
# CONFIG += debug
# CONFIG -= release

!contains(CONFIG,debug) {
QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRTD /NODEFAULTLIB:MSVCR71D /NODEFAULTLIB:MSVCP71D
}
contains(CONFIG,debug) {
QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCR71 /NODEFAULTLIB:MSVCP71 /NODEFAULTLIB:libc
}
