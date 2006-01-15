kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdecore/network ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(ksendbugmail_SRCS
main.cpp
smtp.cpp
)

kde4_automoc(${ksendbugmail_SRCS})

kde4_add_executable(ksendbugmail ${ksendbugmail_SRCS})

target_link_libraries(ksendbugmail  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/bin ksendbugmail )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
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
#bin_PROGRAMS = ksendbugmail
#INCLUDES= -I$(srcdir)/.. -I$(top_srcdir)/kdecore/network $(all_includes)
#
#ksendbugmail_SOURCES = main.cpp smtp.cpp
#ksendbugmail_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#ksendbugmail_LDADD = ../../libkio.la
#
#METASOURCES = AUTO
