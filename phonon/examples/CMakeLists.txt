kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(crossfade_SRCS crossfade.cpp )

kde4_automoc(${crossfade_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(crossfade ${crossfade_SRCS})

target_link_libraries(crossfade  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(all_includes)
#LDADD = ../libphononcore.la
#AM_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#
#METASOURCES = AUTO
#
#check_PROGRAMS = crossfade
#
#crossfade_SOURCES = crossfade.cpp
