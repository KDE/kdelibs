
INCLUDES =  -I$(srcdir)/.. $(all_includes)

libkmidincludedir = $(includedir)/libkmid
libkmidinclude_HEADERS = midiout.h player.h track.h midimapper.h \
	midfile.h dattypes.h midistat.h deviceman.h synthout.h \
	fmout.h gusout.h alsaout.h voiceman.h notearray.h mt32togm.h \
	midispec.h libkmid.h

lib_LTLIBRARIES = libkmid.la
libkmid_la_SOURCES  = midiout.cc player.cc track.cc midimapper.cc \
	midfile.cc dattypes.cc midistat.cc deviceman.cc synthout.cc \
	fmout.cc gusout.cc alsaout.cc voiceman.cc mt32togm.cc notearray.cc \
	libkmid.cc

libkmid_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 0:95  -no-undefined
libkmid_la_LIBADD = $(LIBASOUND) ../kdecore/libkdecore.la

DOXYGEN_REFERENCES = kdecore
include ../admin/Doxyfile.am
