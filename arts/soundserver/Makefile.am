
INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow
MCOPINC   = -I$(srcdir) -I$(top_srcdir)/arts/flow
MCOPIDL   = $(top_builddir)/arts/mcopidl/mcopidl
FLOWLIBS  = $(top_builddir)/arts/flow/libartsflow.la

####### install idl files

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = soundserver.h soundserver.idl kmedia2.h kmedia2.idl

####### libsoundserver_idl, libkmedia2_idl (idl files as library)

lib_LTLIBRARIES = libsoundserver_idl.la libkmedia2_idl.la

libsoundserver_idl_la_LIBADD = libkmedia2_idl.la \
                          $(top_builddir)/arts/flow/libartsflow_idl.la
libsoundserver_idl_la_LDFLAGS = -no-undefined
libsoundserver_idl_la_SOURCES = soundserver.cc
libkmedia2_idl_la_SOURCES = kmedia2.cc
libkmedia2_idl_la_LIBADD  = $(top_builddir)/arts/mcop/libmcop.la
libkmedia2_idl_la_LDFLAGS = -no-undefined

###### "real" programs

bin_PROGRAMS = artsd artsplay artscat artswrapper

artsd_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artsd_SOURCES = simplesoundserver_impl.cc artsd.cc wavplayobject_impl.cc

artscat_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artscat_SOURCES = artscat.cc

artsplay_LDADD = libsoundserver_idl.la
artsplay_SOURCES = artsplay.cc

artswrapper_OBJS = artswrapper.o

bd = `echo '$(bindir)' |sed -e "s,\\",,g"`

artswrapper.o: $(srcdir)/artswrapper.c
	$(CC) -c $(DEFS) $(INCLUDES) -DEXECUTE=\"$(bd)/artsd\" \
			$(srcdir)/artswrapper.c

DISTCLEANFILES = soundserver.cc soundserver.h kmedia2.h kmedia2.cc

####### Build rules

soundserver.h: $(top_srcdir)/arts/soundserver/soundserver.idl kmedia2.h $(MCOPIDL)
	$(MCOPIDL) $(MCOPINC) $(top_srcdir)/arts/soundserver/soundserver.idl

soundserver.cc: soundserver.h

kmedia2.h: $(top_srcdir)/arts/soundserver/kmedia2.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/soundserver/kmedia2.idl

kmedia2.cc: kmedia2.h

##
## MP3 stuff (experimental)
##
#
#SUBDIRS = players
#
#lib_LTLIBRARIES = libsoundserver_idl.la libkmedia2_idl.la libmp3playobject.la
#artsd_LDADD = libmp3playobject.la libsoundserver_idl.la $(FLOWLIBS)
#
#libmp3playobject_la_SOURCES = mp3playobject.cc mp3playobject_impl.cpp artsPlugin.cpp
#libmp3playobject_la_LIBADD =  $(FLOWLIBS) \
    #players/inputPlugin/libinputplugin.la \
    #players/outPlugin/liboutplugin.la \
    #players/playerPlugin/libplayerplugin.la \
    #players/mpegplay/libmpegplay.la \
    #players/mpgplayer/libmpgplayer.la \
    #players/tplay/libtplay.la \
    #players/splay/libsplay.la \
    #players/audio/libaudio.la \
    #-lXext -lX11 -lpthread

#mp3playobject.h: $(top_srcdir)/arts/soundserver/mp3playobject.idl $(MCOPIDL)
	#$(MCOPIDL) $(MCOPINC) $(top_srcdir)/arts/soundserver/mp3playobject.idl

#mp3playobject.cc: mp3playobject.h
