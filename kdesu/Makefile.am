## Makefile.am for libkdesu

INCLUDES = $(all_includes) -I$(top_srcdir)/kio/

lib_LTLIBRARIES = libkdesu.la
libkdesu_la_SOURCES = pty.cpp client.cpp process.cpp kcookie.cpp su.cpp ssh.cpp stub.cpp
libkdesu_la_LDFLAGS = -version-info 1:0 -no-undefined $(all_libraries) 
libkdesu_la_LIBADD = $(LIB_KDECORE)

kdesudir = $(includedir)/kdesu
kdesu_HEADERS = defaults.h client.h process.h pty.h kcookie.h su.h ssh.h stub.h

bin_PROGRAMS = kdesu_stub
kdesu_stub_SOURCES = kdesu_stub.c
