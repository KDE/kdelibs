
noinst_LIBRARIES = libplayer.a

libplayer_a_SOURCES  = midiout.cc player.cc track.cc midimapper.cc \
	 midfile.cc dattypes.cc midistat.cc deviceman.cc synthout.cc

noinst_PROGRAMS = consolekmid
consolekmid_SOURCES = main.cc
consolekmid_LDADD = libplayer.a

