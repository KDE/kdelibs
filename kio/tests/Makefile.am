#    This file is part of the KDE libraries
#    Copyright (C) 1997 David Faure <faure@kde.org>

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

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio $(all_includes)
LDADD = $(top_builddir)/kio/libksycoca.la
LDFLAGS = $(all_libraries) $(KDE_RPATH)

check_PROGRAMS = ksycocatest getalltest kruntest ktartest \
                 kioslavetest kdirwatchtest kshredtest speed kurifiltertest \
                 ktradertest kdefaultprogresstest kclipboardtest kmimemagictest \
                 kfiltertest kiopassdlgtest kscantest kdirlistertest \
                 previewtest kionetrctest

METASOURCES = AUTO

## libkio only
speed_LDADD = $(top_builddir)/kio/libkio.la
speed_SOURCES = speed.cpp
kioslavetest_LDADD = $(top_builddir)/kio/libkio.la
kioslavetest_SOURCES = kioslavetest.cpp
kshredtest_LDADD = $(top_builddir)/kio/libkio.la
kshredtest_SOURCES = kshredtest.cpp
kdefaultprogresstest_LDADD = $(top_builddir)/kio/libkio.la
kdefaultprogresstest_SOURCES = kdefaultprogresstest.cpp
kclipboardtest_LDADD = $(top_builddir)/kio/libkio.la
kclipboardtest_SOURCES = kclipboardtest.cpp
kionetrctest_LDADD = $(top_builddir)/kio/libkio.la
kionetrctest_SOURCES = kionetrctest.cpp

## libksycoca
kiopassdlgtest_SOURCES = kiopassdlgtest.cpp
kurifiltertest_SOURCES = kurifiltertest.cpp
ksycocatest_SOURCES = ksycocatest.cpp
getalltest_SOURCES = getalltest.cpp
kruntest_SOURCES = kruntest.cpp
kdirwatchtest_SOURCES = kdirwatchtest.cpp kdirwatchtest.h
ktradertest_SOURCES = ktradertest.cpp
kmimemagictest_SOURCES = kmimemagictest.cpp
kfiltertest_SOURCES = kfiltertest.cpp
kscantest_SOURCES = kscantest.cpp
kdirlistertest_SOURCES = kdirlistertest.cpp kdirlistertest.h
previewtest_SOURCES = previewtest.cpp previewtest.h
ktartest_SOURCES = ktartest.cpp

#Needs kfile, so commented out
#kopenwithtest_SOURCES = kopenwithtest.cpp
#kopenwithtest_LDADD = $(LIB_KFILE)

