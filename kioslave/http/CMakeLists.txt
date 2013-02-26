
project(kioslave-http)

include(ConfigureChecks.cmake)
configure_file(config-kioslave-http.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kioslave-http.h )

find_package(GSSAPI)
set_package_properties(GSSAPI PROPERTIES DESCRIPTION "Allows KIO to make use of certain HTTP authentication services"
                       URL "http://web.mit.edu/kerberos/www"
                       TYPE OPTIONAL
                       PURPOSE "A MIT or HEIMDAL flavor of GSSAPI can be used"
                      )

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


include_directories( ${KDE4_KIO_INCLUDES}
                     ${ZLIB_INCLUDE_DIR}
                     ${Solid_INCLUDE_DIRS}
                     ${KArchive_INCLUDE_DIRS}
                     ${KWidgetsAddons_INCLUDE_DIRS}
                     ${KCoreAddons_INCLUDE_DIRS}
                     ${ki18n_SOURCE_DIR}/src
                     ${ki18n_BINARY_DIR}/src
)
include_directories( ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/kio/httpfilter )

add_subdirectory( kcookiejar )
add_subdirectory( tests )

########### next target ###############

set(kio_http_cache_cleaner_KDEINIT_SRCS
   http_cache_cleaner.cpp
   )


kde4_add_kdeinit_executable( kio_http_cache_cleaner NOGUI ${kio_http_cache_cleaner_KDEINIT_SRCS})

target_link_libraries(kdeinit_kio_http_cache_cleaner ${KDE4_KIO_LIBS} ${ZLIB_LIBRARY} ${KCoreAddons_LIBRARIES} ki18n)

install(TARGETS kdeinit_kio_http_cache_cleaner ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kio_http_cache_cleaner kdeinit_kio_http_cache_cleaner )
install(TARGETS kio_http_cache_cleaner DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### next target ###############

# kio/httpfilter/Makefile.am: httpfilter

set(kio_http_PART_SRCS
   http.cpp
   httpauthentication.cpp
   ${httpfilter_STAT_SRCS}
   ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc
   )


kde4_add_plugin(kio_http ${kio_http_PART_SRCS})

target_link_libraries(kio_http ${KArchive_LIBRARIES} ${KDE4_KIO_LIBS} ${Solid_LIBRARIES} kntlm ${ZLIB_LIBRARY} ${KCoreAddons_LIBRARIES} ki18n)
if(GSSAPI_FOUND)
  target_link_libraries(kio_http ${GSSAPI_LIBS} )
endif(GSSAPI_FOUND)

install(TARGETS  kio_http  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES
    http_cache_cleaner.desktop
    http.protocol
    https.protocol
    webdav.protocol
    webdavs.protocol
  DESTINATION  ${SERVICES_INSTALL_DIR} )


