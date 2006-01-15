kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdeprint ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(helpviewer_SRCS
helpwindow.cpp
main.cpp
richpage.cpp
)

kde4_automoc(${helpviewer_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(helpviewer ${helpviewer_SRCS})

target_link_libraries(helpviewer  ${QT_AND_KDECORE_LIBS} kdeprint )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(drawdemo_SRCS
drawdemo.cpp
)

kde4_automoc(${drawdemo_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(drawdemo ${drawdemo_SRCS})

target_link_libraries(drawdemo  ${QT_AND_KDECORE_LIBS} kdeprint )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kdeprint -I$(top_srcdir) $(all_includes)
#
#check_PROGRAMS = helpviewer drawdemo
#
#helpviewer_SOURCES = helpwindow.cpp main.cpp richpage.cpp
#helpviewer_METASOURCES = helpwindow.moc
#helpviewer_LDADD = $(top_builddir)/kdeprint/libkdeprint.la
#helpviewer_LDFLAGS = $(all_libraries)
#
#drawdemo_SOURCES = drawdemo.cpp
#drawdemo_METASOURCES = drawdemo.moc
#drawdemo_LDADD = $(top_builddir)/kdeprint/libkdeprint.la
#drawdemo_LDFLAGS = $(all_libraries)
#
#EXTRA_DIST = home.xpm back.xpm forward.xpm
