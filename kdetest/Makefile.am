#	This file is part of the KDE libraries
#    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
#              (C) 1997-1998 Stephan Kulow (coolo@kde.org)

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
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

top_realbuilddir = `cd $(top_builddir) && pwd`


INCLUDES = -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui \
          -I$(top_srcdir)/kformula -I$(top_srcdir)/kio\
	  -I$(top_srcdir)/kfile -I$(top_srcdir)/khtml \
	  $(all_includes)

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = kformulatest \
		khtmltest \
		kspelltest \
		kmemtest \
		kidlservertest kidlclienttest \
		dcopkonqtest


noinst_HEADERS = KIDLTest.h

# METASOURCES = AUTO

dcopkonqtest_SOURCES = dcopkonqtest.cpp
dcopkonqtest_LDADD = $(LIB_KDECORE)

kformulatest_SOURCES = kformulatest.cpp
kformulatest_LDADD = $(LIB_KFORMULA)
khtmltest_SOURCES = khtmltest.cpp
khtmltest_LDADD = $(LIB_KHTML)
kspelltest_SOURCES = kspelltest.cpp
kspelltest_LDADD = $(top_builddir)/kspell/libkspell.la
kmemtest_SOURCES = kmemtest.cpp
kmemtest_LDADD = $(LIB_KDECORE)
kidlservertest_SOURCES = KIDLTest.cpp KIDLTest_skel.cpp
kidlservertest_LDADD = $(LIB_KDECORE)
kidlclienttest_SOURCES = KIDLTestClient.cpp KIDLTest_stub.cpp
kidlclienttest_LDADD = $(LIB_KDECORE)

KIDLTest.kidl: KIDLTest.h
	( ../dcop/dcopidl/dcopidl KIDLTest.h >KIDLTest.kidl; ../dcop/dcopidl2cpp/dcopidl2cpp KIDLTest.kidl )
KIDLTest_skel.cpp: KIDLTest.kidl
KIDLTest_stub.cpp: KIDLTest.kidl
KIDLTest_stub.h: KIDLTest.kidl

