## KDE/libkfm/Makefile.am
##
## (C) 1997 Stephan Kulow

MOC = @MOC@
INCLUDES = @QT_INCLUDES@ @X_INCLUDES@ -I../kdecore
CLEANFILES = *.moc

lib_LTLIBRARIES = libkfm.la
libkfm_la_LDFLAGS = -version-info 0:1  -rpath $(libdir)

libkfm_la_SOURCES = kfm.cpp kfmclient_ipc.cpp kfmclient_ipc2.cpp ipc.cpp
BUILT_SOURCES = kfm.moc kfmclient_ipc.moc
include_HEADERS = kfm.h kfmclient_ipc.h ipc.h

########## Meta objects ##########

%.moc: %.h
	$(MOC) $< -o $@

kfm.cpp: kfm.moc
kfmclient_ipc.cpp: kfmclient_ipc.moc













