#    This file is part of the KDE libraries
#    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

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

INCLUDES = -I$(top_srcdir) $(all_includes)

METASOURCES = AUTO

check_PROGRAMS = statetest guitest

guitest_SOURCES = main.cpp testwidget.cpp
guitest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
guitest_LDADD   = $(LIB_KPARTS) $(top_builddir)/kdem2m/libkdem2mcore.la

statetest_SOURCES = statetest.cpp
statetest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
statetest_LDADD   = $(top_builddir)/kdem2m/libkdem2mcore.la
