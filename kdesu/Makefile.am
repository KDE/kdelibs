## Makefile.am for libkdesu

INCLUDES = $(all_includes) -I$(top_srcdir)/kio/

lib_LTLIBRARIES = libkdesu.la
libkdesu_la_SOURCES = kdesu_pty.cpp client.cpp process.cpp kcookie.cpp su.cpp ssh.cpp stub.cpp
libkdesu_la_LDFLAGS = -version-info 1:0 -no-undefined $(all_libraries) 
libkdesu_la_LIBADD = $(LIB_KDECORE) $(LIBUTIL)

kdesudir = $(includedir)/kdesu
kdesu_HEADERS = defaults.h client.h process.h kdesu_pty.h kcookie.h su.h ssh.h stub.h

install-data-local:
	-rm -f $(DESTDIR)$(includedir)/kdesu/pty.h
	ln -s kdesu_pty.h $(DESTDIR)$(includedir)/kdesu/pty.h
uninstall-local:
	-rm -f $(DESTDIR)$(includedir)/kdesu/pty.h

bin_PROGRAMS = kdesu_stub
kdesu_stub_SOURCES = kdesu_stub.c
