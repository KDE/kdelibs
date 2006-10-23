
project(dnssd)

macro_optional_find_package(DNSSD)
macro_bool_to_01(DNSSD_FOUND HAVE_DNSSD)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-dnssd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-dnssd.h )


include_directories( ${KDE4_KDECORE_INCLUDES} )
if (HAVE_DNSSD)
  include_directories( ${DNSSD_INCLUDE_DIR} )
endif (HAVE_DNSSD)


########### next target ###############

set(kdnssd_LIB_SRCS
   remoteservice.cpp
   responder.cpp
   servicebase.cpp
   publicservice.cpp
   query.cpp
   domainbrowser.cpp
   servicebrowser.cpp
   )

kde4_automoc(${kdnssd_LIB_SRCS})

set( kdnssd_KCFG_SRCS settings.kcfgc )

kde4_add_kcfg_files(kdnssd_LIB_SRCS ${kdnssd_KCFG_SRCS})

kde4_add_library(kdnssd SHARED ${kdnssd_LIB_SRCS})

if (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDE4_KDECORE_LIBS} ${DNSSD_LIBRARIES} )
else (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDE4_KDECORE_LIBS})
endif (HAVE_DNSSD)

set_target_properties(kdnssd PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install(TARGETS kdnssd  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES domainbrowser.h query.h remoteservice.h   publicservice.h servicebase.h servicebrowser.h ${CMAKE_CURRENT_BINARY_DIR}/settings.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/dnssd )




