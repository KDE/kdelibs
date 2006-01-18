kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kde3support ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kunittest_LIB_SRCS
runner.cpp
tester.cpp
)

kde4_automoc(${kunittest_LIB_SRCS})

kde4_add_library(kunittest SHARED ${kunittest_LIB_SRCS})

target_link_libraries(kunittest  ${QT_AND_KDECORE_LIBS} )

set_target_properties(kunittest PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kunittest )


########### next target ###############

set(kunittestmodrunner_SRCS
modrunner.cpp
)

kde4_automoc(${kunittestmodrunner_SRCS})

kde4_add_executable(kunittestmodrunner ${kunittestmodrunner_SRCS})

target_link_libraries(kunittestmodrunner  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kunittestmodrunner )


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kde3support $(all_includes)
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = libkunittest.la
#libkunittest_la_SOURCES = runner.cpp tester.cpp
#libkunittest_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#libkunittest_la_LIBADD = $(LIB_KDECORE)
#
#libkunittestinclude_HEADERS = runner.h tester.h module.h
#libkunittestincludedir = $(includedir)/kunittest
#
#bin_PROGRAMS = kunittestmodrunner
#kunittestmodrunner_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kunittestmodrunner_LDADD = libkunittest.la $(LIB_KDECORE)
#kunittestmodrunner_SOURCES = modrunner.cpp
#
## The check_ target makes sure we don't install the modules,
## -rpath $(libdir) assures a shared library is created (used to be done by $(KDE_CHECK_PLUGIN)).
#check_LTLIBRARIES = kunittest_samplemodule.la
#kunittest_samplemodule_la_SOURCES = samplemodule.cpp
#kunittest_samplemodule_la_LIBADD = libkunittest.la
#kunittest_samplemodule_la_LDFLAGS = -module $(KDE_PLUGIN) -rpath $(libdir) $(all_libraries)
#
#check-local:
#	./kunittestmodrunner
#
#include $(top_srcdir)/admin/Doxyfile.am
