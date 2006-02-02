kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${HSPELL_INCLUDE_DIR}  )


########### next target ###############

set(kspell_hspell_PART_SRCS
kspell_hspellclient.cpp
kspell_hspelldict.cpp
)

kde4_automoc(${kspell_hspell_PART_SRCS})

kde4_add_plugin(kspell_hspell ${kspell_hspell_PART_SRCS})

target_link_libraries(kspell_hspell  ${QT_AND_KDECORE_LIBS} kspell2 ${HSPELL_LIBRARY} ${ZLIB_LIBRARIES} )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kspell_hspell )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES kspell_hspell.desktop )

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
#kde_module_LTLIBRARIES = kspell_hspell.la
#
#kspell_hspell_la_SOURCES = kspell_hspellclient.cpp kspell_hspelldict.cpp
#
#kspell_hspell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
#kspell_hspell_la_LIBADD = ../../ui/libkspell2.la -lhspell -lz
#
#service_DATA = kspell_hspell.desktop
#servicedir = $(kde_servicesdir)
