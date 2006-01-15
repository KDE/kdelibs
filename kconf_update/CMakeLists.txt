kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kconf_update_KDEINIT_SRCS
kconf_update.cpp
)

kde4_automoc(${kconf_update_KDEINIT_SRCS})

kde4_add_klm( kconf_update ${kconf_update_KDEINIT_SRCS})

target_link_libraries(kdeinit_kconf_update  ${QT_AND_KDECORE_LIBS} )

install_targets(/lib kdeinit_kconf_update )

target_link_libraries( kconf_update kdeinit_kconf_update )
install_targets(/bin kconf_update )

########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 2001 Waldo Bastian <bastian@kde.org>
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
#INCLUDES= -I../kded $(all_includes)
#
#bin_PROGRAMS = 
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kconf_update.la
#kconf_update_la_SOURCES = kconf_update.cpp
#kconf_update_la_LIBADD = $(LIB_KDECORE)
#kconf_update_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#METASOURCES = AUTO
#
## Internal
## noinst_HEADERS = kconf_update.h
#
#include $(top_srcdir)/admin/Doxyfile.am
#
