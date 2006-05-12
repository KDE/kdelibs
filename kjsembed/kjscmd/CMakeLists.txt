
project(kjsembed-kjscmd)

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/kjsembed/kjsembed )


########### next target ###############

set(kjscmd_SRCS kjscmd.cpp )

kde4_automoc(${kjscmd_SRCS})

qt4_add_resources(kjscmd_SRCS kjscmd.qrc )

kde4_add_executable(kjscmd ${kjscmd_SRCS})

target_link_libraries(kjscmd  ${KDE4_KDECORE_LIBS} kjsembed )

install(TARGETS kjscmd DESTINATION bin)


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I../kjsembed
#
#bin_PROGRAMS = kjscmd 
#
#kjscmd_SOURCES = kjscmd.cpp kjscmd.qrc
#kjscmd_LDFLAGS = $(all_libraries) 
#kjscmd_LDADD = ../kjsembed/libkjsembed.la 
#
##man_MANS = kjscmd.1
#
##xdg_apps_DATA = kjscmd.desktop
#
