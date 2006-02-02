kde4_header()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dcop-path.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/dcop-path.h )

add_subdirectory( KDE-ICE )
add_subdirectory( dcopidlng )
add_subdirectory( dcopidl2cpp )
add_subdirectory( client )
add_subdirectory( tests )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

add_definitions(-DICE_t -DTRANS_SERVER -DTRANS_CLIENT -DANSICPP)

if (UNIX)
   # unix
   add_definitions(-DUNIXCONN)
else (UNIX)
   # windows
   add_definitions(-DDCOP_EXPORT="" -DSOCKCONN -DTCPCONN -DMAKE_DCOP_LIB -FIkdelibs_global_win.h)
endif (UNIX)


# dcop/KDE-ICE/Makefile.am: kICE

set(kICE_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/accept.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/authutil.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/connect.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/error.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/getauth.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/iceauth.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/listen.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/listenwk.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/locking.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/misc.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/ping.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/process.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/protosetup.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/register.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/replywait.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/setauth.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/shutdown.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/watch.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/transport.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/globals.c
)

########### next target ###############

set(dcopserver_shutdown_SRCS
dcopserver_shutdown.c
)

#kde4_automoc(${dcopserver_shutdown_SRCS})

kde4_add_executable(dcopserver_shutdown ${dcopserver_shutdown_SRCS})

target_link_libraries(dcopserver_shutdown )

install_targets(/bin dcopserver_shutdown )


########### next target ###############

set(testdcop_SRCS
testdcop.cpp
)

kde4_automoc(${testdcop_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testdcop ${testdcop_SRCS})

target_link_libraries(testdcop  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testdcopc_SRCS
testdcopc.c
dcopc.c
${kICE_STAT_SRCS}
)

kde4_automoc(${testdcopc_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testdcopc ${testdcopc_SRCS})

target_link_libraries(testdcopc  ${QT_AND_KDECORE_LIBS})

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(DCOP_LIB_SRCS
dcopstub.cpp
dcopref.cpp
dcopobject.cpp
dcopclient.cpp
${kICE_STAT_SRCS}
)

kde4_automoc(${DCOP_LIB_SRCS})

kde4_add_library(DCOP SHARED ${DCOP_LIB_SRCS})

target_link_libraries(DCOP  ${QT_QTCORE_LIBRARY})

if (WIN32)
   target_link_libraries(DCOP kdewin32 ws2_32)
endif (WIN32)


set_target_properties(DCOP PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${KDE4_LIB_INSTALL_DIR} DCOP )


########### next target ###############

set(dcopserver_KDEINIT_SRCS
dcopserver.cpp
dcopsignals.cpp
)

kde4_automoc(${dcopserver_KDEINIT_SRCS})

kde4_add_klm( dcopserver ${dcopserver_KDEINIT_SRCS})

target_link_libraries(kdeinit_dcopserver  ${QT_QTCORE_LIBRARY} DCOP)

install_targets(${KDE4_LIB_INSTALL_DIR} kdeinit_dcopserver )

target_link_libraries( dcopserver kdeinit_dcopserver )
install_targets(/bin dcopserver )

########### install files ###############
install_files( /include FILES dcopclient.h dcopobject.h dcopstub.h dcopref.h dcoptypes.h kdatastream.h )


kde4_create_doxygen_docs( EXCLUDE KDE-ICE testdcop.h testdcop.cpp  )

kde4_footer()



#original Makefile.am contents follow:

## This Makefile.am was taken from the kdelibs distribution and
## modified for DCOP.  Preston Brown (Oct 10, 1999)
##
## Copyright (c) 1999 Preston Brown <pbrown@kde.org>
## Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
## AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
## CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#SUBDIRS = KDE-ICE . dcopidlng dcopidl2cpp client tests
#KDE_OPTIONS = qtonly
#
#INCLUDES = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#bin_PROGRAMS = dcopserver_shutdown
#check_PROGRAMS = testdcop testdcopc
# # dcop_deadlock_test
#
#lib_LTLIBRARIES  = libDCOP.la
#libDCOP_la_LIBADD = KDE-ICE/libkICE.la $(LIB_QTCORE)
#libDCOP_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)
#libDCOP_la_NMCHECK = $(srcdir)/libDCOP.nmcheck
#libDCOP_la_NMCHECKWEAK = $(srcdir)/libDCOP_weak.nmcheck $(top_srcdir)/kdecore/libqt-mt
#
#include_HEADERS = dcopclient.h dcopobject.h dcopstub.h dcopref.h dcoptypes.h kdatastream.h
#noinst_HEADERS = version.h dcopglobal.h dcopserver.h testdcop.h dcopsignals.h dcop-path.h
#
#METASOURCES = AUTO
#
# dcopstub needs to go first for --enable-final to work
#libDCOP_la_SOURCES = dcopstub.cpp dcopref.cpp dcopobject.cpp dcopclient.cpp
#
#kdeinit_LTLIBRARIES = dcopserver.la
#dcopserver_la_LDFLAGS = -module -avoid-version $(all_libraries)
#dcopserver_la_SOURCES = dcopserver.cpp dcopsignals.cpp
#dcopserver_la_LIBADD  = libDCOP.la
#
#dcopserver_shutdown_SOURCES = dcopserver_shutdown.c
#
#testdcop_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#testdcop_SOURCES = testdcop.cpp
#testdcop_LDADD = libDCOP.la
#
##dcop_deadlock_test_LDFLAGS = $(KDE_RPATH) $(all_libraries)
##dcop_deadlock_test_SOURCES = dcop_deadlock_test.cpp
##dcop_deadlock_test_LDADD = ../kdecore/libkdecore.la
#
#testdcopc_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#testdcopc_SOURCES = testdcopc.c dcopc.c
#testdcopc_LDADD =  KDE-ICE/libkICE.la $(LIBSM)
#
#EXTRA_DIST = HOWTO ICE Mainpage.dox
#
##DOXYGEN_REFERENCES = kdecore
#DOXYGEN_EXCLUDE = KDE-ICE testdcop.h testdcop.cpp
#include ../admin/Doxyfile.am
