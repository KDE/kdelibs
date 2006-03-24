kde4_header()

add_subdirectory( ui ) 

########### next target ###############

set(phononifaces_LIB_SRCS backend.cpp base.cpp )

kde4_automoc(${phononifaces_LIB_SRCS})

kde4_add_library(phononifaces SHARED ${phononifaces_LIB_SRCS})

target_link_libraries(phononifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(phononifaces PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} phononifaces )


########### install files ###############

install_files( /include/phonon/ifaces FILES  abstractmediaproducer.h mediaobject.h soundcardcapture.h  		       audiopath.h backend.h abstractaudiooutput.h  		       audiooutput.h audiodataoutput.h  		       videopath.h abstractvideooutput.h videoeffect.h audioeffect.h  		       bytestream.h base.h volumefadereffect.h )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
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
#
#SUBDIRS = . ui
#
#lib_LTLIBRARIES = libphononifaces.la
#
#phononincludedir = $(includedir)/phonon/ifaces
#phononinclude_HEADERS = abstractmediaproducer.h mediaobject.h soundcardcapture.h \
#		       audiopath.h backend.h abstractaudiooutput.h \
#		       audiooutput.h audiodataoutput.h \
#		       videopath.h abstractvideooutput.h videoeffect.h audioeffect.h \
#		       bytestream.h base.h volumefadereffect.h
#
#libphononifaces_la_SOURCES = backend.cpp base.cpp
#libphononifaces_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_LDFLAGS) $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
#libphononifaces_la_LIBADD = $(LIB_KDECORE)
#
#METASOURCES = AUTO
