#	This file is part of the KDE libraries
#    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License version 2 as published by the Free Software Foundation.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

INCLUDES = -I$(srcdir)/../libltdl/ $(all_includes)

SUBDIRS = . tests

bin_PROGRAMS = kdeinit kdeinit_wrapper kshell kdeinit_shutdown lnusertemp kwrapper klauncher kioslave
lib_LTLIBRARIES = klauncher.la

kdeinit_SOURCES = kinit.cpp setproctitle.cpp

# NOTE: We link against all common libraries even if we don't need them ourselves.
kdeinit_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
                  $(KDE_RPATH)
kdeinit_LDADD = $(LIB_DCOP) $(LIB_DMALLOC) $(LIB_KPARTS) $(LIB_KSYCOCA) \
		$(LIB_KDEUI) $(LIB_KIO) $(LIB_KDECORE) $(LIBUTIL)

kioslave_SOURCES = kioslave.cpp
kioslave_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
                  $(KDE_RPATH)
kioslave_LDADD = $(LIB_DCOP) $(LIB_DMALLOC) $(LIB_KIO) $(LIB_KDECORE)

kdeinit_wrapper_SOURCES = wrapper.c
kdeinit_wrapper_LDADD = $(LIBSOCKET)

kshell_SOURCES = shell.c
kshell_LDADD = $(LIBSOCKET)

kdeinit_shutdown_SOURCES = wrapper.c
kdeinit_shutdown_LDADD = $(LIBSOCKET)

lnusertemp_SOURCES = lnusertemp.c 
lnusertemp_LDFLAGS = $(KDE_RPATH)

kwrapper_SOURCES = kwrapper.c
kwrapper_LDADD = $(LIBSOCKET)

klauncher_la_LDFLAGS = $(all_libraries) -module -avoid-version
klauncher_la_LIBADD = $(LIB_KSYCOCA)
klauncher_la_SOURCES = klauncher.cpp klauncher_main.cpp autostart.cpp

klauncher_LDFLAGS =  $(all_libraries) $(KDE_RPATH)
klauncher_LDADD = klauncher.la
klauncher_SOURCES = dummy.cpp

METASOURCES = AUTO

noinst_HEADERS = klauncher.h autostart.h klauncher_cmds.h setproctitle.h

kwrapper.o: wrapper.c

shell.o: wrapper.c

dummy.cpp:
	echo > dummy.cpp
