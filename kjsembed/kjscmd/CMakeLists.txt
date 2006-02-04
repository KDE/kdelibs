kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kjscmd_SRCS
kjscmd.cpp
)

kde4_automoc(${kjscmd_SRCS})

set( kjscmd_QRC
kjscmd.qrc
)

qt4_add_resources(kjscmd_SRCS ${kjscmd_QRC} )

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
