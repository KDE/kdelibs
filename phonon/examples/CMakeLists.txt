
########### next target ###############

set(crossfade_SRCS crossfade.cpp )

kde4_automoc(${crossfade_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(crossfade ${crossfade_SRCS})

target_link_libraries(crossfade  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############





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
