
SUBDIRS = mcopclass
INCLUDES = -I$(top_srcdir)/arts/mcop $(all_includes)
CLOSURE = $(libartsflow_la_OBJECTS) $(libartsflow_idl_la_OBJECTS)

####### Files

lib_LTLIBRARIES = libartsflow_idl.la libartsflow.la

libartsflow_idl_la_SOURCES = artsflow.cc
libartsflow_idl_la_LDFLAGS = -no-undefined
libartsflow_idl_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la

libartsflow_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la libartsflow_idl.la $(LIBAUDIOFILE) $(LIBASOUND) $(LIBAUDIOIO) $(LIBOSSAUDIO) $(LIBAUDIONAS) -lm
libartsflow_la_LDFLAGS = -no-undefined
libartsflow_la_SOURCES =  synth_play_impl.cc \
  synthschedule.cc audiosubsys.cc \
  pipebuffer.cc convert.cc synth_wave_sin_impl.cc synth_frequency_impl.cc \
  synth_multi_add_impl.cc synth_add_impl.cc synth_mul_impl.cc \
  synth_play_wav_impl.cc stdsynthmodule.cc cache.cc asyncschedule.cc \
  bytestreamtoaudio_impl.cc stereovolumecontrol_impl.cc \
  stereoeffectstack_impl.cc fft.c stereofftscope_impl.cc virtualports.cc \
  bus.cc audiomanager_impl.cc synth_record_impl.cc resample.cc \
  audioio.cc audioiooss.cc audioioalsa.cc audioionull.cc audioiolibaudioio.cc \
  audioiosun.cc audioioaix.cc audioionas.cc audioioalsa9.cc cpuinfo.cc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = artsflow.h audiosubsys.h cache.h \
  cachedwav.h convert.h pipebuffer.h stdsynthmodule.h \
  synthschedule.h fft.h artsflow.idl audioio.h resample.h \
  cpuinfo.h

DISTCLEANFILES = artsflow.cc artsflow.h artsflow.mcoptype artsflow.mcopclass

######## Build rules

artsflow.cc: artsflow.h
artsflow.mcoptype: artsflow.h
artsflow.mcopclass: artsflow.h
artsflow.h: $(top_srcdir)/arts/flow/artsflow.idl $(MCOPIDL)
	$(MCOPIDL) -t $(top_srcdir)/arts/flow/artsflow.idl

######## install idl typeinfo files

mcoptypedir = $(libdir)/mcop
mcoptype_DATA = artsflow.mcoptype artsflow.mcopclass
