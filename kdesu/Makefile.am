## Makefile.am for libkdesu

INCLUDES = -I$(top_srcdir)/kio/ $(all_includes)

lib_LTLIBRARIES = libkdesu.la
libkdesu_la_SOURCES = client.cpp process.cpp kcookie.cpp su.cpp ssh.cpp stub.cpp kdesu_pty.cpp
libkdesu_la_LDFLAGS = -version-info 5:0:1 -no-undefined $(all_libraries) 
libkdesu_la_LIBADD = $(LIB_KDECORE) $(LIBUTIL)

kdesudir = $(includedir)/kdesu
kdesu_HEADERS = defaults.h client.h process.h kdesu_pty.h kcookie.h su.h ssh.h stub.h

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(kdesudir)
	-rm -f $(DESTDIR)$(kdesudir)/pty.h
	ln -s kdesu_pty.h $(DESTDIR)$(kdesudir)/pty.h

uninstall-local:
	-rm -f $(DESTDIR)$(kdesudir)/pty.h

bin_PROGRAMS = kdesu_stub
kdesu_stub_SOURCES = kdesu_stub.c
