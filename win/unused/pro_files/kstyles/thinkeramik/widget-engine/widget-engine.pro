include( ../../common.pro )

TARGET		= thinkeramik$$KDEBUG

unix { 
	INCLUDEPATH += moc
	#force finding libraries in current dir (for installer, etc):
	QMAKE_LFLAGS += -Wl,-rpath,.
}

system( bash kmoc )

SOURCES = \
colorutil.cpp \
gradients.cpp \
thinkeramik.cpp \
pixmaploader.cpp

contains( KW_CONFIG, release ) {
	system( bash ./genemb.sh _rel )
}

!contains( KW_CONFIG, release ) {
	system( bash ./genemb.sh )
}

