add_subdirectory( icons )


include_directories( ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(kimproxy_LIB_SRCS
  kimproxy.cpp
  kimadaptor.cpp
  kimiface.cpp
)

kde4_automoc(${kimproxy_LIB_SRCS})

kde4_add_library(kimproxy SHARED ${kimproxy_LIB_SRCS})

target_link_libraries(kimproxy  ${KDE4_KIO_LIBS} )

set_target_properties(kimproxy PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

########### install files ###############

install(TARGETS kimproxy  DESTINATION ${LIB_INSTALL_DIR} )
install( FILES kimproxy.h kimproxyiface.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )
