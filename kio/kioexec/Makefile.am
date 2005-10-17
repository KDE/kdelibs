AM_CPPFLAGS = $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

bin_PROGRAMS = kioexec

kioexec_SOURCES = main.cpp
kioexec_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kioexec_LDADD   = $(LIB_KIO)

noinst_HEADERS = main.h
METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kioexec_SOURCES) -o $(podir)/kioexec.pot

install-exec-local:
	@rm -f $(DESTDIR)$(bindir)/kfmexec
	@$(LN_S) kioexec $(DESTDIR)$(bindir)/kfmexec

include $(top_srcdir)/admin/Doxyfile.am
