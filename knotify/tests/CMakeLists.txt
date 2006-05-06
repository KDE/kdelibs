kde4_header()

project(knotifytest)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../config )

########### knotifytest ###############

set(knotifytest_SRCS main.cpp knotifytestwindow.cpp )

kde4_add_ui_files(knotifytest_SRCS knotifytestview.ui)

kde4_automoc(${knotifytest_SRCS})

kde4_add_executable(knotifytest ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} knotifyconfig )

install(TARGETS knotifytest DESTINATION bin)


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
