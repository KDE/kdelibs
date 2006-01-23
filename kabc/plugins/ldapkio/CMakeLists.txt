kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabc_ldapkio_LIB_SRCS
resourceldapkio.cpp
resourceldapkioconfig.cpp
resourceldapkioplugin.cpp
)

kde4_automoc(${kabc_ldapkio_LIB_SRCS})

kde4_add_library(kabc_ldapkio SHARED ${kabc_ldapkio_LIB_SRCS})

target_link_libraries(kabc_ldapkio  ${QT_AND_KDECORE_LIBS} kabc kio )

set_target_properties(kabc_ldapkio PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kabc_ldapkio )


########### next target ###############

set(kabc_ldapkio_PART_SRCS
dummy.cpp
)

kde4_automoc(${kabc_ldapkio_PART_SRCS})

kde4_add_plugin(kabc_ldapkio ${kabc_ldapkio_PART_SRCS})

target_link_libraries(kabc_ldapkio  ${QT_AND_KDECORE_LIBS} )

install_targets(/lib/kde4 kabc_ldapkio )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR}/kresources/kabc FILES ldapkio.desktop )
install_files( /include/kabc FILES resourceldapkio.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = resourceldapkioconfig.h
#
#lib_LTLIBRARIES = libkabc_ldapkio.la
#libkabc_ldapkio_la_SOURCES = resourceldapkio.cpp resourceldapkioconfig.cpp
#libkabc_ldapkio_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
#libkabc_ldapkio_la_LIBADD	= $(top_builddir)/kabc/libkabc.la $(LIB_KIO)
#libkabc_ldapkio_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h
#
#kde_module_LTLIBRARIES = kabc_ldapkio.la
#kabc_ldapkio_la_SOURCES = resourceldapkioplugin.cpp
#kabc_ldapkio_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
#kabc_ldapkio_la_LIBADD	= libkabc_ldapkio.la
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabc_ldapkio.pot
#
#kabcincludedir = $(includedir)/kabc
#kabcinclude_HEADERS = resourceldapkio.h
#
#servicedir = $(kde_servicesdir)/kresources/kabc
#service_DATA = ldapkio.desktop
#
#resourceldapkioplugin.lo: ../../addressee.h
