kde4_header()

########### next target ###############

set(phonon_fakeui_PART_SRCS videowidget.cpp backend.cpp )

kde4_automoc(${phonon_fakeui_PART_SRCS})

kde4_add_plugin(phonon_fakeui ${phonon_fakeui_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} phonon_fakeui )

target_link_libraries(phonon_fakeui  ${KDE4_KDECORE_LIBS} phononifacesui kio )

install_targets(${LIB_INSTALL_DIR}/kde4 phonon_fakeui )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License version 2 as published by the Free Software Foundation.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
##    Boston, MA 02111-1307, USA.
#
#INCLUDES = -I$(top_srcdir) $(all_includes)
#AM_CXXFLAGS = -DNDEBUG
#
#kde_module_LTLIBRARIES = phonon_fakeui.la
#
#phonon_fakeui_la_SOURCES = videowidget.cpp backend.cpp
#phonon_fakeui_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#phonon_fakeui_la_LIBADD = ../../../ifaces/ui/libphononifacesui.la $(LIB_KIO)
#
#METASOURCES = AUTO
