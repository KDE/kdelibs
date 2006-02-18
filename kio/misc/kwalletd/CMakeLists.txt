kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kwallet/backend ${CMAKE_SOURCE_DIR}/kwallet/client ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kded_kwalletd_PART_SRCS
   kwalletd.cpp
   ktimeout.cpp
)

kde4_automoc(${kded_kwalletd_PART_SRCS})

set( kded_kwalletd_UI3
   kwalletwizard.ui3
   kbetterthankdialogbase.ui3
)

kde4_add_ui3_files(kded_kwalletd_PART_SRCS ${kded_kwalletd_UI3} )

set( kded_kwalletd_DCOP_SKEL_SRCS kwalletd.h )

kde4_add_dcop_skels(kded_kwalletd_PART_SRCS ${kded_kwalletd_DCOP_SKEL_SRCS})

kde4_add_plugin(kded_kwalletd ${kded_kwalletd_PART_SRCS})

target_link_libraries(kded_kwalletd  ${QT_AND_KDECORE_LIBS} kio kwalletbackend kwalletclient )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kwalletd )
install_targets(${PLUGIN_INSTALL_DIR} kded_kwalletd )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kded FILES kwalletd.desktop )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 2002 George Staikos <staikos@kde.org>
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
#INCLUDES= -I$(srcdir)/.. -I$(top_srcdir) -I$(top_srcdir)/kwallet/backend -I$(top_builddir)/kwallet/backend -I$(top_srcdir)/kwallet/client -I$(top_builddir)/kwallet/client $(all_includes)
#
#kde_module_LTLIBRARIES = kded_kwalletd.la
#
#kded_kwalletd_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kded_kwalletd_la_LIBADD = $(LIB_KIO) ../../../kwallet/backend/libkwalletbackend.la ../../../kwallet/client/libkwalletclient.la $(LIB_KDED)
#kded_kwalletd_la_SOURCES = kwalletd.cpp kwalletd.skel ktimeout.cpp kwalletwizard.ui3 kbetterthankdialogbase.ui3
#
#METASOURCES = AUTO
#
#noinst_HEADERS = kwalletd.h ktimeout.h kwalletwizard.ui.h
#
#services_DATA = kwalletd.desktop
#servicesdir = $(kde_servicesdir)/kded
#
#kwalletwizard.lo: kwalletwizard.ui3 kwalletwizard.ui.h
#kbetterthankdialogbase.lo: kbetterthankdialogbase.ui3 kbetterthankdialogbase.ui.h
