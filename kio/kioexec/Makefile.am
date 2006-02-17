AM_CPPFLAGS = $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

bin_PROGRAMS = kioexec

kioexec_SOURCES = main.cpp
kioexec_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kioexec_LDADD   = $(LIB_KIO)

noinst_HEADERS = main.h
METASOURCES = AUTO

### TODO: this is already in kio.pot
messages:
	$(XGETTEXT) rc.cpp main.cpp -o $(podir)/kioexec.pot

include $(top_srcdir)/admin/Doxyfile.am
