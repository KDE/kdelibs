# This Makefile.am was taken from the kdelibs distribution and
# modified for DCOP.  Preston Brown (Oct 10, 1999)
#
# Copyright (c) 1999 Preston Brown <pbrown@kde.org>
# Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SUBDIRS = KDE-ICE . dcopidl dcopidl2cpp client
KDE_OPTIONS = qtonly

INCLUDES=  $(all_includes)

bin_PROGRAMS = dcopserver dcopserver_shutdown
check_PROGRAMS = testdcop testdcopc

lib_LTLIBRARIES  = libDCOP.la dcopserver.la
libDCOP_la_LIBADD = KDE-ICE/libkICE.la $(LIB_QT)
libDCOP_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)

include_HEADERS = dcopclient.h dcopobject.h dcopstub.h dcopref.h dcoptypes.h kdatastream.h
noinst_HEADERS = version.h dcopglobal.h dcopserver.h dcop-doc.h testdcop.h dcopsignals.h dcop-path.h

# we don't use AUTO here, because dcop-doc.h would be mocified (it contains
# Q_OBJECT), and this would give an ugly warning
libDCOP_la_METASOURCES = dcopclient.moc
dcopserver_la_METASOURCES = dcopserver.moc
testdcop_METASOURCES = testdcop.moc

# dcopstub needs to go first for --enable-final to work
libDCOP_la_SOURCES = dcopstub.cpp dcopref.cpp dcopobject.cpp dcopclient.cpp

dcopserver_la_LDFLAGS = -module -avoid-version $(all_libraries)
dcopserver_la_SOURCES = dcopserver.cpp dcopsignals.cpp
dcopserver_la_LIBADD   = libDCOP.la

dcopserver_LDFLAGS = $(KDE_RPATH) $(all_libraries)
dcopserver_SOURCES = dummy.cpp
dcopserver_LDADD   = dcopserver.la

dcopserver_shutdown_SOURCES = dcopserver_shutdown.c

testdcop_LDFLAGS = $(KDE_RPATH) $(all_libraries)
testdcop_SOURCES = testdcop.cpp
testdcop_LDADD = ../kdecore/libkdecore.la

testdcopc_LDFLAGS = $(KDE_RPATH) $(all_libraries)
testdcopc_SOURCES = testdcopc.c dcopc.c
testdcopc_LDADD =  KDE-ICE/libkICE.la $(LIBSM)

EXTRA_DIST = HOWTO ICE

dummy.cpp:
	echo > dummy.cpp

DOXYGEN_REFERENCES = kdecore
DOXYGEN_EXCLUDE = KDE-ICE/*
include ../admin/Doxyfile.am
