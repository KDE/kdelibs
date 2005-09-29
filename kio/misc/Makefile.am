#    This file is part of the KDE libraries
#    Copyright (C) 1997 Torben Weis (weis@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

INCLUDES=  -I$(srcdir)/.. -I$(top_srcdir) -I$(srcdir)/../kssl -I../kssl $(all_includes) $(SSL_INCLUDES)

SUBDIRS = . ksendbugmail kpac kdesasl kssld kfile kwalletd kntlm

lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kio_uiserver.la
bin_PROGRAMS = kmailservice ktelnetservice

observer_DIR = $(top_srcdir)/kio/kio

kio_uiserver_la_SOURCES = uiserver.cpp uiserver.skel observer.stub
kio_uiserver_la_LIBADD = $(LIB_KIO)
kio_uiserver_la_LDFLAGS = $(all_libraries) -module -avoid-version

METASOURCES = AUTO

kde_services_DATA = kio_uiserver.desktop

kmailservice_SOURCES = kmailservice.cpp
kmailservice_LDADD = $(LIB_KDECORE)
kmailservice_LDFLAGS = $(KDE_RPATH) $(all_libraries)

ktelnetservice_SOURCES = ktelnetservice.cpp
ktelnetservice_LDADD = $(LIB_KDEUI)
ktelnetservice_LDFLAGS = $(KDE_RPATH) $(all_libraries)

protocol_DATA = kmailservice.protocol telnet.protocol rlogin.protocol rtsp.protocol ssh.protocol \
		mms.protocol mmst.protocol mmsu.protocol pnm.protocol rtspt.protocol rtspu.protocol
protocoldir = $(kde_servicesdir)

kiomiscdir = $(includedir)/kio
kiomisc_HEADERS = uiserver.h


bin_SCRIPTS = fileshareset
install-exec-local:
	@-rm -f $(DESTDIR)$(bindir)/filesharelist
	@$(LN_S) fileshareset $(DESTDIR)$(bindir)/filesharelist
	@(chown root $(DESTDIR)$(bindir)/fileshareset && chmod 4755 $(DESTDIR)$(bindir)/fileshareset) || echo "Was not able to make fileshareset setuid root"

