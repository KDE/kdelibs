INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop
LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la
MCOPIDL = $(top_builddir)/arts/mcopidl/mcopidl

####### Files

lib_LTLIBRARIES = libartsflow.la

libartsflow_la_SOURCES = artsflow.cc synth_play_impl.cc artsserver_impl.cc \
  executionman_impl.cc execrequest.cc synthschedule.cc debug.cc audiosubsys.cc \
  pipebuffer.cc convert.cc synth_wave_sin_impl.cc synth_frequency_impl.cc \
  synth_multi_add_impl.cc synth_add_impl.cc synth_mul_impl.cc \
  synth_play_wav_impl.cc stdsynthmodule.cc cache.cc utils.c

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = artsflow.h artsserver_impl.h audiosubsys.h cache.h \
  convert.h debug.h execrequest.h pipebuffer.h stdsynthmodule.h \
  synthschedule.h utils.h

DISTCLEANFILES = $(SRCMETA) artsflow.cc artsflow.h

######## Build rules

artsflow.cc: artsflow.h
artsflow.h: $(top_srcdir)/arts/flow/artsflow.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/flow/artsflow.idl

#
# -frepo closure (experimental)
#
closure: $(libartsflow_la_OBJECTS)
	@echo "Building -frepo compliant library closure"
	@echo "int main() {}" > closure.cc
	$(CXXLINK) -o closure $(libartsflow_la_OBJECTS) closure.cc
	@echo "closure timestamp" > closure
	@rm -f closure.cc
