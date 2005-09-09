#    This file is part of the KDE libraries
#
#    Copyright (C) 2005 David Faure <faure@kde.org>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
#    Boston, MA 02110-1301, USA.

lib_LTLIBRARIES = libqttestlib.la

libqttestlib_la_SOURCES = qtestcase.cpp qtestlog.cpp qtesttable.cpp qtestdata.cpp qtestresult.cpp qasciikey.cpp qplaintestlogger.cpp qxmltestlogger.cpp qsignaldumper.cpp
libqttestlib_la_LIBADD = $(LIB_QTCORE)
libqttestlib_la_LDFLAGS = $(KDE_RPATH) $(KDE_MT_LDFLAGS) $(all_libraries) -no-undefined -version-info 6:0:2

AM_CPPFLAGS = $(all_includes) -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_ASCII -DQT_NO_DATASTREAM -DQTESTLIB_MAKEDLL

include_HEADERS = QtTest/qttest_global.h QtTest/qttestcase.h QtTest/qttesttable.h QtTest/qttestdata.h

