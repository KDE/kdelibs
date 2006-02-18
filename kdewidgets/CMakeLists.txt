kde4_header()

project(kdewidgets)

add_subdirectory( pics ) 
add_subdirectory( tests ) 

include_directories( ${CMAKE_SOURCE_DIR}/kfile ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(makekdewidgets_SRCS makekdewidgets.cpp )

kde4_automoc(${makekdewidgets_SRCS})

kde4_add_executable(makekdewidgets ${makekdewidgets_SRCS})

target_link_libraries(makekdewidgets  ${QT_AND_KDECORE_LIBS} kdecore )

install_targets(/bin makekdewidgets )


########### next target ###############


add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
  COMMAND ${CMAKE_BINARY_DIR}/bin/makekdewidgets
  ARGS -o ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets 
  DEPENDS kde.widgets makekdewidgets)

set(kdewidgets_PART_SRCS
   classpreviews.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
)

qt4_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.moc)

kde4_automoc(classpreviews.cpp)

set( kdewidgets_QRC kdewidgets.qrc )

qt4_add_resources(kdewidgets_PART_SRCS ${kdewidgets_QRC} )

kde4_add_plugin(kdewidgets ${kdewidgets_PART_SRCS})

target_link_libraries(kdewidgets  ${QT_AND_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/designer kdewidgets )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/designer kdewidgets )


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
#SUBDIRS=. pics tests
#AM_CXXFLAGS = -DQT_PLUGIN
#
#INCLUDES = -I$(top_srcdir)/kfile $(all_includes)
#
#bin_PROGRAMS = makekdewidgets
#makekdewidgets_SOURCES = makekdewidgets.cpp
#makekdewidgets_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#makekdewidgets_LDADD = $(LIB_QT) ../kdecore/libkdecore.la
#
#kde_widget_LTLIBRARIES = kdewidgets.la
#kdewidgets_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
#kdewidgets_la_LIBADD = $(LIB_KIO) $(LIB_KABC)
#kdewidgets_la_SOURCES = classpreviews.cpp kdewidgets.cpp kdewidgets.qrc
#
#kdewidgets.cpp: $(srcdir)/kde.widgets makekdewidgets
#	./makekdewidgets -o kdewidgets.cpp $(srcdir)/kde.widgets
#
#kdewidgets.lo: kdewidgets.moc
#kdewidgets.moc: kdewidgets.cpp
#	$(MOC) -o kdewidgets.moc kdewidgets.cpp
#
#CLEANFILES = kdewidgets.cpp kdewidgets.moc
#
