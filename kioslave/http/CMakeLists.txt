kde4_header()

add_subdirectory( kcookiejar ) 

include_directories( ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/kdecore/network ${CMAKE_SOURCE_DIR}/kio/httpfilter ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_http_cache_cleaner_KDEINIT_SRCS
http_cache_cleaner.cpp
)

kde4_automoc(${kio_http_cache_cleaner_KDEINIT_SRCS})

kde4_add_klm( kio_http_cache_cleaner ${kio_http_cache_cleaner_KDEINIT_SRCS})

target_link_libraries(kdeinit_kio_http_cache_cleaner  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/lib kdeinit_kio_http_cache_cleaner )

target_link_libraries( kio_http_cache_cleaner kdeinit_kio_http_cache_cleaner )
install_targets(/bin kio_http_cache_cleaner )

########### next target ###############

set(kio_http_PART_SRCS
http.cc
${httpfilter_STAT_SRCS}
)

kde4_automoc(${kio_http_PART_SRCS})

kde4_add_plugin(kio_http ${kio_http_PART_SRCS})

target_link_libraries(kio_http  ${QT_AND_KDECORE_LIBS} kio kntlm )

install_targets(/lib/kde4 kio_http )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES http_cache_cleaner.desktop http.protocol https.protocol                webdav.protocol webdavs.protocol )

kde4_create_doxygen_docs(  )

kde4_footer()



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
