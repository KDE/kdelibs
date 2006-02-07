kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio/kssl ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kded_kssld_PART_SRCS
kssld.cpp
)

kde4_automoc(${kded_kssld_PART_SRCS})

set( kded_kssld_DCOP_SKEL_SRCS
kssld.h
)

kde4_add_dcop_skels(kded_kssld_PART_SRCS ${kded_kssld_DCOP_SKEL_SRCS})

kde4_add_plugin(kded_kssld ${kded_kssld_PART_SRCS})

kde4_create_libtool_file(kded_kssld /kde4/ )

target_link_libraries(kded_kssld  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kded_kssld )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR}/kded FILES kssld.desktop )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 2001 George Staikos <staikos@kde.org>
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
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#INCLUDES= -I$(srcdir)/.. -I$(top_srcdir) $(SSL_INCLUDES) -I$(top_srcdir)/kio/kssl -I$(top_builddir)/kio/kssl $(all_includes)
#
#kde_module_LTLIBRARIES = kded_kssld.la
#
#kded_kssld_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kded_kssld_la_LIBADD = $(LIB_KIO) $(LIB_KDED)
#kded_kssld_la_SOURCES = kssld.cpp kssld.skel
#
#METASOURCES = AUTO
#
#noinst_HEADERS = kssld.h
#
#services_DATA = kssld.desktop
#servicesdir = $(kde_servicesdir)/kded
#
