
#	This file is part of the KDE libraries
#    Copyright (C) 2000 Waldo Bastian (bastian@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License version 2 as published by the Free Software Foundation.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this library; see the file COPYING.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

AM_CPPFLAGS = -DQT_NO_ASCII_CAST

SUBDIRS = . tests

INCLUDES= $(all_includes)

lib_LTLIBRARIES = libkprint.la

libkprint_la_LDFLAGS = -no-undefined -version-info 3:0
libkprint_la_LIBADD = ../kdeui/libkdeui.la -laps
libkprint_la_SOURCES = kprintdialog.cpp kprinter.cpp

include_HEADERS = kprintdialog.h kprinter.h

noinst_HEADERS = 

libkprint_la_METASOURCES = AUTO

