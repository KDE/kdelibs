kde4_header()

add_subdirectory( ui ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(phonon_fake_PART_SRCS 
   abstractmediaproducer.cpp 
   abstractaudiooutput.cpp 
   audiodataoutput.cpp 
   audioeffect.cpp 
   audiooutput.cpp 
   audiopath.cpp 
   bytestream.cpp 
   mediaobject.cpp 
   avcapture.cpp 
   videoeffect.cpp 
   videopath.cpp 
   backend.cpp 
   volumefadereffect.cpp )

kde4_automoc(${phonon_fake_PART_SRCS})

kde4_add_plugin(phonon_fake ${phonon_fake_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} phonon_fake )

target_link_libraries(phonon_fake  ${KDE4_KDECORE_LIBS} phononifaces phononcore kio )

install_targets(${LIB_INSTALL_DIR}/kde4 phonon_fake )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/phononbackends FILES  fake.desktop )

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
#SUBDIRS = . ui
#
#INCLUDES = -I$(top_srcdir) $(all_includes)
#AM_CXXFLAGS = -DNDEBUG
#
#kde_module_LTLIBRARIES = phonon_fake.la
#
#phonon_fake_la_SOURCES = abstractmediaproducer.cpp abstractaudiooutput.cpp \
#                         audiodataoutput.cpp \
#                         audioeffect.cpp audiooutput.cpp \
#                         audiopath.cpp bytestream.cpp mediaobject.cpp \
#                         avcapture.cpp videoeffect.cpp videopath.cpp \
#                         backend.cpp volumefadereffect.cpp
#phonon_fake_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#phonon_fake_la_LIBADD = ../../ifaces/libphononifaces.la $(LIB_KIO)
#
#METASOURCES = AUTO
#
#phononbackendfilesdir = $(kde_servicesdir)/phononbackends
#phononbackendfiles_DATA = fake.desktop
#
