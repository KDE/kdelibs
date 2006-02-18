kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kspell_ispell_PART_SRCS
kspell_ispellclient.cpp
kspell_ispelldict.cpp
correct.cpp
good.cpp
hash.cpp
lookup.cpp
makedent.cpp
tgood.cpp
ispell_checker.cpp
)

kde4_automoc(${kspell_ispell_PART_SRCS})

kde4_add_plugin(kspell_ispell ${kspell_ispell_PART_SRCS})

target_link_libraries(kspell_ispell  ${QT_AND_KDECORE_LIBS} kspell2 )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kspell_ispell )
install_targets(${PLUGIN_INSTALL_DIR} kspell_ispell )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kspell_ispell.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = -I$(top_srcdir)/kspell2 -I$(top_srcdir) $(all_includes)
#
## For the future: examine if condensing the tons of *_LDFLAGS variables
## into $(all_libraries) isn't better
#AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)
#
#kde_module_LTLIBRARIES = kspell_ispell.la
#
#kspell_ispell_la_SOURCES = kspell_ispellclient.cpp kspell_ispelldict.cpp \
#	correct.cpp		\
#	good.cpp		\
#	hash.cpp		\
#	lookup.cpp		\
#	makedent.cpp		\
#	tgood.cpp		\
#	ispell_checker.cpp
#
#kspell_ispell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
#kspell_ispell_la_LIBADD = ../../ui/libkspell2.la 
#
#service_DATA = kspell_ispell.desktop
#servicedir = $(kde_servicesdir)
