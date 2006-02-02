kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ASPELL_INCLUDE_DIR}  )


########### next target ###############

set(kspell_aspell_PART_SRCS
kspell_aspellclient.cpp
kspell_aspelldict.cpp
)

kde4_automoc(${kspell_aspell_PART_SRCS})

kde4_add_plugin(kspell_aspell ${kspell_aspell_PART_SRCS})

target_link_libraries(kspell_aspell  ${QT_AND_KDECORE_LIBS} kspell2 ${ASPELL_LIBRARY} )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kspell_aspell )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES kspell_aspell.desktop )

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
#kde_module_LTLIBRARIES = kspell_aspell.la
#
#kspell_aspell_la_SOURCES = kspell_aspellclient.cpp kspell_aspelldict.cpp
#
#kspell_aspell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
#kspell_aspell_la_LIBADD = ../../ui/libkspell2.la -laspell
#
#service_DATA = kspell_aspell.desktop
#servicedir = $(kde_servicesdir)
