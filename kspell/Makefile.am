# This Makefile.am was taken from the kdelibs distribution and modified for
#  kspell.  David Sweet (3/1/98)
#
#	This file is part of the KDE libraries
#    Copyright (C) 1997 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this library; see the file COPYING.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.


INCLUDES=  $(KDE_INCLUDES) $(QT_INCLUDES) $(X_INCLUDES)

DISTCLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES  = libkspell.la
libkspell_la_LDFLAGS = -version-info 2:0

DISTCLEANFILES = kspelldlg.moc kspell.moc ksconfig.moc kprocio.moc

include_HEADERS = kspell.h ksconfig.h kprocio.h kspelldlg.h

libkspell_la_SOURCES = kspelldlg.cpp kspell.cpp ksconfig.cpp kprocio.cpp

kspell.cpp: kspell.moc
kspell.moc: kspell.h
	$(MOC) kspell.h -o kspell.moc

kspelldlg.cpp: kspelldlg.moc
kspelldlg.moc: kspelldlg.h
	$(MOC) kspelldlg.h -o kspelldlg.moc

ksconfig.cpp: ksconfig.moc
ksconfig.moc: ksconfig.h
	$(MOC) ksconfig.h -o ksconfig.moc

kprocio.cpp: kprocio.moc
kprocio.moc: kprocio.h
	$(MOC) kprocio.h -o kprocio.moc
