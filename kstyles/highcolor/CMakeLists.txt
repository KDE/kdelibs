kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(highcolor_PART_SRCS
highcolor.cpp
)

kde4_automoc(${highcolor_PART_SRCS})

kde4_add_plugin(highcolor ${highcolor_PART_SRCS})

target_link_libraries(highcolor  ${QT_AND_KDECORE_LIBS} kdefx )

install_targets(/lib/kde4 highcolor )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
##	This file is part of the KDE libraries
##    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
##			  (C) 1997 Stephan Kulow (coolo@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU General Public License
##    along with this library; see the file COPYING.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#AM_CPPFLAGS = -DQT_PLUGIN
#
#INCLUDES = -I$(top_srcdir)/kdefx $(all_includes)
#noinst_HEADERS = highcolor.h bitmaps.h
#kde_style_LTLIBRARIES = highcolor.la
#highcolor_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#highcolor_la_LIBADD = ../../kdefx/libkdefx.la
#highcolor_la_SOURCES = highcolor.cpp
#highcolor_la_METASOURCES = AUTO
