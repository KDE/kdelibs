
INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop
LDFLAGS  = $(top_builddir)/arts/mcop/libmcop.la
MCOPIDL = $(top_builddir)/arts/mcopidl/mcopidl
CLOSURE = $(libartsflow_la_OBJECTS) $(libartsflow_idl_la_OBJECTS)

####### Files

lib_LTLIBRARIES = libartsflow_idl.la libartsflow.la

libartsflow_idl_la_SOURCES = artsflow.cc
libartsflow_idl_la_LDFLAGS = -no-undefined

libartsflow_la_LIBADD = libartsflow_idl.la $(LIBAUDIOFILE)
libartsflow_la_LDFLAGS = -no-undefined
libartsflow_la_SOURCES =  synth_play_impl.cc \
  synthschedule.cc debug.cc audiosubsys.cc \
  pipebuffer.cc convert.cc synth_wave_sin_impl.cc synth_frequency_impl.cc \
  synth_multi_add_impl.cc synth_add_impl.cc synth_mul_impl.cc \
  synth_play_wav_impl.cc stdsynthmodule.cc cache.cc utils.c asyncschedule.cc \
  bytestreamtoaudio_impl.cc stereovolumecontrol_impl.cc \
  stereoeffectstack_impl.cc fft.c stereofftscope_impl.cc virtualports.cc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = artsflow.h audiosubsys.h cache.h \
  cachedwav.h convert.h debug.h pipebuffer.h stdsynthmodule.h \
  synthschedule.h utils.h fft.h artsflow.idl 

DISTCLEANFILES = artsflow.cc artsflow.h

######## Build rules

artsflow.cc: artsflow.h
artsflow.h: $(top_srcdir)/arts/flow/artsflow.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/flow/artsflow.idl

