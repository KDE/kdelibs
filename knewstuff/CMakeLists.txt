kde4_header()

project(knewstuff)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(knewstuff_LIB_SRCS
   engine.cpp
   entry.cpp
   downloaddialog.cpp
   uploaddialog.cpp
   providerdialog.cpp
   provider.cpp
   knewstuff.cpp
   knewstuffgeneric.cpp
   knewstuffbutton.cpp
   knewstuffsecure.cpp
   security.cpp
   )

kde4_automoc(${knewstuff_LIB_SRCS})

kde4_add_library(knewstuff SHARED ${knewstuff_LIB_SRCS})

target_link_libraries(knewstuff  ${QT_AND_KDECORE_LIBS} kio )

set_target_properties(knewstuff PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${KDE4_LIB_INSTALL_DIR} knewstuff )


########### next target ###############

set(khotnewstuff_SRCS khotnewstuff.cpp )

kde4_automoc(${khotnewstuff_SRCS})

kde4_add_executable(khotnewstuff ${khotnewstuff_SRCS})

target_link_libraries(khotnewstuff  ${QT_AND_KDECORE_LIBS} knewstuff)

install_targets(/bin khotnewstuff )


########### next target ###############

if(KDE4_BUILD_TESTS)

set(testnewstuff_SRCS testnewstuff.cpp )

kde4_automoc(${testnewstuff_SRCS})

kde4_add_executable(testnewstuff ${testnewstuff_SRCS})

target_link_libraries(testnewstuff  ${QT_AND_KDECORE_LIBS} knewstuff)


########### next target ###############

set(ghns_SRCS ghns.cpp )

kde4_automoc(${ghns_SRCS})

kde4_add_executable(ghns ${ghns_SRCS})

target_link_libraries(ghns  ${QT_AND_KDECORE_LIBS} knewstuff)

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( ${KDE4_CONFIG_DIR} FILES khotnewstuffrc )
install_files( /include/knewstuff FILES downloaddialog.h engine.h entry.h knewstuffgeneric.h knewstuff.h providerdialog.h provider.h uploaddialog.h knewstuffbutton.h knewstuffsecure.h )

kde4_install_icons( crystalsvg /share )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = libknewstuff.la
#
#libknewstuff_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
#libknewstuff_la_LIBADD  = $(LIB_KIO)
#libknewstuff_la_SOURCES = engine.cpp entry.cpp downloaddialog.cpp \
#  uploaddialog.cpp providerdialog.cpp provider.cpp knewstuff.cpp \
#  knewstuffgeneric.cpp knewstuffbutton.cpp knewstuffsecure.cpp security.cpp
#
#bin_PROGRAMS = khotnewstuff
#
#khotnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#khotnewstuff_LDADD = libknewstuff.la
#khotnewstuff_SOURCES = khotnewstuff.cpp
#
#EXTRA_PROGRAMS = testnewstuff ghns
#
#testnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testnewstuff_LDADD   = libknewstuff.la
#testnewstuff_SOURCES = testnewstuff.cpp
#
#ghns_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#ghns_LDADD   = libknewstuff.la
#ghns_SOURCES = ghns.cpp
#
#knewstuffdir = $(kde_datadir)/knewstuff
#knewstuff_DATA = types
#
#kde_conf_DATA = khotnewstuffrc
#
#knsdir = $(includedir)/knewstuff
#kns_HEADERS = downloaddialog.h engine.h entry.h knewstuffgeneric.h knewstuff.h providerdialog.h provider.h uploaddialog.h knewstuffbutton.h knewstuffsecure.h
#noinst_HEADERS = testnewstuff.h ghns.h security.h
#
#KDE_ICON=AUTO
#
#include ../admin/Doxyfile.am
#
