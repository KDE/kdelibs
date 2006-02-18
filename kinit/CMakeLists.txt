kde4_header()

project(kdeinit)

add_subdirectory( tests ) 

include_directories( ${CMAKE_SOURCE_DIR}/kdecore ${CMAKE_SOURCE_DIR}/kdecore/network ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeinit_SRCS kinit.cpp setproctitle.cpp )

kde4_automoc(${kdeinit_SRCS})

kde4_add_executable(kdeinit ${kdeinit_SRCS})

target_link_libraries(kdeinit  ${QT_AND_KDECORE_LIBS} kparts )

install_targets(/bin kdeinit )


########### next target ###############

set(kdeinit_wrapper_SRCS wrapper.c )

kde4_add_executable(kdeinit_wrapper ${kdeinit_wrapper_SRCS})

target_link_libraries(kdeinit_wrapper  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kdeinit_wrapper )


########### next target ###############

set(kshell_SRCS shell.c )

kde4_add_executable(kshell ${kshell_SRCS})

target_link_libraries(kshell  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kshell )


########### next target ###############

set(kdeinit_shutdown_SRCS wrapper.c )

kde4_add_executable(kdeinit_shutdown ${kdeinit_shutdown_SRCS})

target_link_libraries(kdeinit_shutdown  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kdeinit_shutdown )


########### next target ###############

set(lnusertemp_SRCS lnusertemp.c )

kde4_add_executable(lnusertemp ${lnusertemp_SRCS})

target_link_libraries(lnusertemp  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin lnusertemp )


########### next target ###############

set(kwrapper_SRCS kwrapper.c )

kde4_add_executable(kwrapper ${kwrapper_SRCS})

target_link_libraries(kwrapper  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kwrapper )


########### next target ###############

set(kioslave_SRCS kioslave.cpp )

kde4_automoc(${kioslave_SRCS})

kde4_add_executable(kioslave ${kioslave_SRCS})

target_link_libraries(kioslave  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/bin kioslave )


########### next target ###############

set(kstartupconfig_SRCS kstartupconfig.cpp )

kde4_automoc(${kstartupconfig_SRCS})

kde4_add_executable(kstartupconfig ${kstartupconfig_SRCS})

target_link_libraries(kstartupconfig  ${QT_AND_KDECORE_LIBS} kdefakes )

install_targets(/bin kstartupconfig )


########### next target ###############

set(kdostartupconfig_SRCS kdostartupconfig.cpp )

kde4_automoc(${kdostartupconfig_SRCS})

kde4_add_executable(kdostartupconfig ${kdostartupconfig_SRCS})

target_link_libraries(kdostartupconfig  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kdostartupconfig )


########### next target ###############

set(klauncher_KDEINIT_SRCS
   klauncher.cpp
   klauncher_main.cpp
   autostart.cpp
   )

kde4_automoc(${klauncher_KDEINIT_SRCS})

kde4_add_kdeinit_executable( klauncher ${klauncher_KDEINIT_SRCS})

target_link_libraries(kdeinit_klauncher  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR} kdeinit_klauncher )

target_link_libraries( klauncher kdeinit_klauncher )
install_targets(/bin klauncher )

########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

##	This file is part of the KDE libraries
##    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
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
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#INCLUDES = -I$(srcdir)/../libltdl/ -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdecore/network $(all_includes) $(KDEINIT_XFT_INCLUDES)
#
#SUBDIRS = . tests
#
#bin_PROGRAMS = kdeinit kdeinit_wrapper kshell kdeinit_shutdown lnusertemp kwrapper kioslave \
#               kstartupconfig kdostartupconfig
#
#lib_LTLIBRARIES = 
#kdeinit_LTLIBRARIES = klauncher.la
#kdeinit_SOURCES = kinit.cpp setproctitle.cpp
#
## NOTE: We link against all common libraries even if we don't need them ourselves.
## For the future: examine if condensing the tons of *_LDFLAGS variables
## into $(all_libraries) isn't better
#AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)
#
#kdeinit_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
#                  $(KDE_RPATH)
#kdeinit_LDADD = $(LIB_KPARTS) 
#
#kioslave_SOURCES = kioslave.cpp
#kioslave_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
#                  $(KDE_RPATH)
#kioslave_LDADD = $(LIB_KIO) 
#
#kdeinit_wrapper_SOURCES = wrapper.c
#kdeinit_wrapper_LDADD = $(LIBSOCKET)
#
#kshell_SOURCES = shell.c
#kshell_LDADD = $(LIBSOCKET)
#
#kdeinit_shutdown_SOURCES = wrapper.c
#kdeinit_shutdown_LDADD = $(LIBSOCKET)
#
#lnusertemp_SOURCES = lnusertemp.c 
#lnusertemp_LDFLAGS = $(KDE_RPATH)
#
#kwrapper_SOURCES = kwrapper.c
#kwrapper_LDADD = $(LIBSOCKET)
#
#klauncher_la_LDFLAGS = $(all_libraries) -module -avoid-version
#klauncher_la_LIBADD = $(LIB_KIO)
#klauncher_la_SOURCES = klauncher.cpp klauncher_main.cpp autostart.cpp
#
#kstartupconfig_SOURCES  = kstartupconfig.cpp
#kstartupconfig_LDADD  = $(top_builddir)/kdecore/libkdefakes.la
#kdostartupconfig_SOURCES = kdostartupconfig.cpp
#kdostartupconfig_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) \
#                           $(KDE_RPATH)
#kdostartupconfig_LDADD  = $(LIB_KDECORE)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = klauncher.h autostart.h klauncher_cmds.h setproctitle.h
#
#kwrapper.o: wrapper.c
#
#shell.o: wrapper.c
#
#dummy.cpp:
#	echo > dummy.cpp
#
#
#include $(top_srcdir)/admin/Doxyfile.am
#
