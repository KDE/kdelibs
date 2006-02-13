kde4_header()

project(kjsembed-kjscmd)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )
include_directories( ${CMAKE_SOURCE_DIR}/kjsembed/kjsembed )


########### next target ###############

set(kjscmd_SRCS kjscmd.cpp )

kde4_automoc(${kjscmd_SRCS})

qt4_add_resources(kjscmd_SRCS kjscmd.qrc )

kde4_add_executable(kjscmd ${kjscmd_SRCS})

target_link_libraries(kjscmd  ${QT_AND_KDECORE_LIBS} kjsembed )

install_targets(/bin kjscmd )


########### install files ###############


kde4_footer()



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
