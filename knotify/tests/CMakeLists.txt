kde4_header()

project(knotifytest)

find_package(KDE4 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

link_directories(${KDE4_LIB_DIR})

include(ConvenienceLibs.cmake)

#include(ManualStuff.cmake)

#include(ConfigureChecks.cmake)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


set( knotifytest_UI knotifytestview.ui )

set(knotifytest_SRCS main.cpp knotifytestwindow.cpp )

kde4_add_ui_files(knotifytest_SRCS ${knotifytest_UI} )

kde4_automoc(${knotifytest_SRCS})

kde4_add_executable(knotifytest ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} knotifyconfig )

install_targets(/bin knotifytest )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/knotifytest FILES  knotifytestui.rc knotifytest.notifyrc )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = -I../client -I../config $(all_includes) -I../client
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#bin_PROGRAMS = knotifytest
#
#knotifytest_SOURCES = main.cpp knotifytestwindow.cpp knotifytestview.ui 
#knotifytest_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO)  $(LIB_QT)
#knotifytest_LDADD = ../config/libknotifyconfig.la
#
#noinst_HEADERS = knotifytestwindow.h
#METASOURCES =	AUTO
#
#rcdir = $(kde_datadir)/knotifytest
#rc_DATA = knotifytestui.rc knotifytest.notifyrc
#
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp *.cpp -o $(podir)/knotifytest.pot
#
