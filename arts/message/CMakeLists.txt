kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(artsmessage_SRCS
artsmessage.cc
)

kde4_automoc(${artsmessage_SRCS})

kde4_add_executable(artsmessage ${artsmessage_SRCS})

target_link_libraries(artsmessage  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(/bin artsmessage )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#bin_PROGRAMS = artsmessage
#
#artsmessage_SOURCES = artsmessage.cc
#
#artsmessage_LDADD = ../../kdeui/libkdeui.la
#artsmessage_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KDE_MT_LDFLAGS)
