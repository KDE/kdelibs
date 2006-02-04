INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(kjscmd_SRCS
kjscmd.cpp
)

KDE3_AUTOMOC(${kjscmd_SRCS})

KDE3_ADD_EXECUTABLE(kjscmd ${kjscmd_SRCS})

TARGET_LINK_LIBRARIES(kjscmd  ${QT_AND_KDECORE_LIBS} kjsembed )

INSTALL_TARGETS(/bin kjscmd )


########### install files ###############


KDE3_PLACEHOLDER()



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
