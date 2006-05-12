add_subdirectory( icons )

include_directories( ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(kimproxy_LIB_SRCS
kimproxy.cpp
)

kde4_automoc(${kimproxy_LIB_SRCS})

set( kimproxy_DCOP_SKEL_SRCS
kimproxyiface.h
)

kde4_add_dcop_skels(kimproxy_LIB_SRCS ${kimproxy_DCOP_SKEL_SRCS})

set( kimproxy_DCOP_STUB_SRCS
${CMAKE_SOURCE_DIR}/interfaces/kimproxy/interface/kimiface.h
)

kde4_add_dcop_stubs(kimproxy_LIB_SRCS ${kimproxy_DCOP_STUB_SRCS})

kde4_add_library(kimproxy SHARED ${kimproxy_LIB_SRCS})

target_link_libraries(kimproxy  ${KDE4_KIO_LIBS} )

set_target_properties(kimproxy PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

########### install files ###############

install_targets(${LIB_INSTALL_DIR} kimproxy )
install_files( /include FILES kimproxy.h kimproxyiface.h )
