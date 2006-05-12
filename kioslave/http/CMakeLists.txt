
project(kioslave-http)

macro_optional_find_package(GSSAPI)

if(GSSAPI_FOUND)
    set(HAVE_LIBGSSAPI 1)
    if(GSSAPI_FLAVOR STREQUAL "MIT")
        set(GSSAPI_MIT 1)
    else(GSSAPI_FLAVOR STREQUAL "MIT")
        set(GSSAPI_MIT 0)
    endif(GSSAPI_FLAVOR STREQUAL "MIT")

    include_directories( ${GSSAPI_INCS} )

else(GSSAPI_FOUND)
    set(HAVE_LIBGSSAPI 0)
    set(GSSAPI_MIT 0)
endif(GSSAPI_FOUND)

configure_file(config-gssapi.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-gssapi.h )


include_directories( ${KDE4_KIO_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/kio/httpfilter )

add_subdirectory( kcookiejar )

########### next target ###############

set(kio_http_cache_cleaner_KDEINIT_SRCS
http_cache_cleaner.cpp
)

kde4_automoc(${kio_http_cache_cleaner_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kio_http_cache_cleaner ${kio_http_cache_cleaner_KDEINIT_SRCS})

target_link_libraries(kdeinit_kio_http_cache_cleaner  ${KDE4_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR} kdeinit_kio_http_cache_cleaner )

target_link_libraries( kio_http_cache_cleaner kdeinit_kio_http_cache_cleaner )
install(TARGETS kio_http_cache_cleaner DESTINATION bin)

########### next target ###############

# kio/httpfilter/Makefile.am: httpfilter

set(kio_http_PART_SRCS
http.cc
${httpfilter_STAT_SRCS}
${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc
)

kde4_automoc(${kio_http_PART_SRCS})

kde4_add_plugin(kio_http ${kio_http_PART_SRCS})

target_link_libraries(kio_http  ${KDE4_KDECORE_LIBS} kio kntlm )
if(GSSAPI_FOUND)
  target_link_libraries(kio_http ${GSSAPI_LIBS} )
endif(GSSAPI_FOUND)

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kio_http )
install_targets(${PLUGIN_INSTALL_DIR}  kio_http )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES http_cache_cleaner.desktop http.protocol https.protocol                webdav.protocol webdavs.protocol )




#original Makefile.am contents follow:

#SUBDIRS = kcookiejar
#
#INCLUDES= -I$(top_srcdir)/interfaces -I$(top_srcdir)/kdecore/network -I$(top_srcdir)/kio/httpfilter $(all_includes) $(GSSAPI_INCS)
#AM_LDFLAGS = $(all_libraries) $(GSSAPI_RPATH)
#
######## Files
#
#bin_PROGRAMS=
#lib_LTLIBRARIES=
#kdeinit_LTLIBRARIES = kio_http_cache_cleaner.la
#kde_module_LTLIBRARIES = kio_http.la
#
#kio_http_la_SOURCES = http.cc
#kio_http_la_METASOURCES = AUTO
#kio_http_la_LIBADD = $(LIB_KIO) $(top_builddir)/kio/httpfilter/libhttpfilter.la $(top_builddir)/kio/misc/kntlm/libkntlm.la
#kio_http_la_LDFLAGS = $(all_libraries) $(GSSAPI_RPATH) -module $(KDE_PLUGIN) $(GSSAPI_LIBS)
#
#kio_http_cache_cleaner_la_SOURCES = http_cache_cleaner.cpp
#kio_http_cache_cleaner_la_LIBADD  = $(LIB_KIO)
#kio_http_cache_cleaner_la_LDFLAGS = -module -avoid-version
#
#noinst_HEADERS = http.h
#
#kdelnkdir = $(kde_servicesdir)
#kdelnk_DATA = http_cache_cleaner.desktop http.protocol https.protocol \
#              webdav.protocol webdavs.protocol
#
#include $(top_srcdir)/admin/Doxyfile.am
