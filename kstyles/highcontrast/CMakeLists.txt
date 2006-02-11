kde4_header()

add_subdirectory( config ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(highcontrast_PART_SRCS highcontrast.cpp )

kde4_automoc(${highcontrast_PART_SRCS})

kde4_add_plugin(highcontrast ${highcontrast_PART_SRCS})

target_link_libraries(highcontrast  ${QT_AND_KDECORE_LIBS} kdefx )

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR} highcontrast )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} highcontrast )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kstyle/themes FILES highcontrast.themerc )

kde4_footer()



#original Makefile.am contents follow:

#
##	This file is part of the KDE libraries
##    Copyright (C) 2005 Olaf Schmidt (ojschmidt@kde.org)
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
#AUTOMAKE_OPTIONS = foreign
#
#SUBDIRS = config
#
#KDE_CXXFLAGS = -DQT_PLUGIN
#
#INCLUDES = $(all_includes)
#
#stylercdir = $(kde_datadir)/kstyle/themes
#stylerc_DATA = highcontrast.themerc
#
#EXTRA_DIST = $(stylerc_DATA)
#
#noinst_HEADERS = highcontrast.h
#
#kde_style_LTLIBRARIES = highcontrast.la
#highcontrast_la_SOURCES = highcontrast.cpp
#highcontrast_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#highcontrast_la_LIBADD = ../../kdefx/libkdefx.la
#highcontrast_la_METASOURCES = AUTO
#
#DISTCLEANFILES = $(highcontrast_la_METASOURCES)
