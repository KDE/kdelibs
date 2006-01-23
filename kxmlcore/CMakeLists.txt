kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kxmlcore_LIB_SRCS
HashTable.cpp
)

kde4_automoc(${kxmlcore_LIB_SRCS})

kde4_add_library(kxmlcore SHARED ${kxmlcore_LIB_SRCS})

target_link_libraries(kxmlcore  ${QT_AND_KDECORE_LIBS} )

set_target_properties(kxmlcore PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(/lib kxmlcore )


########### install files ###############
install_files( /include/kxmlcore FILES HashMap.h         HashTraits.h         HashMapPtrSpec.h           SharedPtr.h          HashSet.h         HashFunctions.h         HashTable.h  )


kde4_create_doxygen_docs( REFERENCES kdecore  )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Lesser General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Lesser General Public License for more details.
#
##    You should have received a copy of the GNU Lesser General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#lib_LTLIBRARIES = libkxmlcore.la
#
#libkxmlcore_la_SOURCES = HashTable.cpp
#
#kxmlcoreincludedir = $(includedir)/kxmlcore
#kxmlcoreinclude_HEADERS = HashMap.h \
#			HashTraits.h \
#			HashMapPtrSpec.h \
#			SharedPtr.h \
#			HashSet.h \
#			HashFunctions.h \
#			HashTable.h
#
#libkxmlcore_la_LDFLAGS = -version-info 3:0:2 -no-undefined $(USER_LDFLAGS) $(all_libraries)
#
#DOXYGEN_REFERENCES = kdecore
#include ../admin/Doxyfile.am
