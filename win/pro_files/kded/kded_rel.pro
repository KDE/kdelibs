#release gui version

QMAKE_CXXFLAGS += -DKBUILDSYCOCA_GUI

!contains(KW_CONFIG,release) {
	KW_CONFIG = debug windows
	KDEBUG=_d
}

include( "kded.pro" )

TARGET		= kbuildsycocaw #gui version
