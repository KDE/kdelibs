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
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

INCLUDES = -I$(srcdir)/../libltdl/ $(all_includes) $(KDEINIT_XFT_INCLUDES)

SUBDIRS = . tests

bin_PROGRAMS = kdeinit kdeinit_wrapper kshell kdeinit_shutdown lnusertemp kwrapper kioslave \
               kstartupconfig kdostartupconfig start_kdeinit

lib_LTLIBRARIES = 
kdeinit_LTLIBRARIES = klauncher.la
kdeinit_SOURCES = kinit.cpp setproctitle.cpp

# NOTE: We link against all common libraries even if we don't need them ourselves.
# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

kdeinit_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
                  $(KDE_RPATH)
kdeinit_LDADD = $(LIB_KPARTS) 

kioslave_SOURCES = kioslave.cpp
kioslave_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
                  $(KDE_RPATH)
kioslave_LDADD = $(LIB_KIO) 

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
klauncher_la_LIBADD = $(LIB_KIO)
klauncher_la_SOURCES = klauncher.cpp klauncher_main.cpp autostart.cpp

kstartupconfig_SOURCES  = kstartupconfig.cpp
kstartupconfig_LDADD  = $(top_builddir)/kdecore/libkdefakes.la
kdostartupconfig_SOURCES = kdostartupconfig.cpp
kdostartupconfig_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
                           $(KDE_RPATH)
kdostartupconfig_LDADD  = $(LIB_KDECORE)

start_kdeinit_SOURCES = start_kdeinit.c
kdeinitpath = $(bindir)/kdeinit
start_kdeinit_CFLAGS = $(KDE_USE_FPIE) -DEXECUTE=\"$(kdeinitpath)\"
start_kdeinit_LDFLAGS = $(KDE_USE_PIE)

METASOURCES = AUTO

noinst_HEADERS = klauncher.h autostart.h klauncher_cmds.h setproctitle.h

kwrapper.o: wrapper.c

shell.o: wrapper.c

dummy.cpp:
	echo > dummy.cpp

if KDEINIT_SETUID
# start_kdeinit needs to be installed setuid root on Linux
install-exec-hook:
	@(chown 0 $(DESTDIR)$(bindir)/start_kdeinit && chmod 4755 $(DESTDIR)$(bindir)/start_kdeinit) || echo "Please make start_kdeinit setuid root" >&2
	@echo ""
	@echo "start_kdeinit is by default installed on Linux with a set SETUID root bit!"
	@echo "This is needed to prevent kdeinit from being killed by a bad heuristic in the OOM-killer when running out of memory."
	@echo ""
endif

include $(top_srcdir)/admin/Doxyfile.am
