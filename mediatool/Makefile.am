
lib_LTLIBRARIES = libmediatool.la
#
# Programs to compile, Manpages to install and Versions
#
MEDIATOOLVERSION=	0.0
MEDIATOOLPATCH	 =	6

libmediatool_la_SOURCES =	chunk.c eventcounter.c mediatypes.c tools.c \
			connect.c filename.c playlist.c kaudio.cpp

include_HEADERS= 	mediatool.h kaudio.h

libmediatool_la_LDFLAGS = -version-info 0:6 -rpath $(libdir)
