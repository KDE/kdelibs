
INCLUDES = $(all_includes)

libkmidincludedir = $(includedir)/libkmid
libkmidinclude_HEADERS = midiout.h player.h track.h midimapper.h \
	midfile.h dattypes.h midistat.h deviceman.h synthout.h \
	fmout.h gusout.h voiceman.h notearray.h mt32togm.h midispec.h \
	libkmid.h

lib_LTLIBRARIES = libkmid.la
libkmid_la_SOURCES  = midiout.cc player.cc track.cc midimapper.cc \
	 midfile.cc dattypes.cc midistat.cc deviceman.cc synthout.cc \
	 fmout.cc gusout.cc voiceman.cc mt32togm.cc notearray.cc libkmid.cc

libkmid_la_CXXFLAGS  = -fno-rtti -Os
libkmid_la_LDFLAGS = -fno-rtti -version-info 0:95 --strip-unneeded

DISTCLEANFILES = ./.deps/* ./.deps/.P
