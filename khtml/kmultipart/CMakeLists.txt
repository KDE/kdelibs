
include_directories( ${CMAKE_SOURCE_DIR}/khtml
                     ${CMAKE_BINARY_DIR}/khtml
                     ${CMAKE_SOURCE_DIR}/kio/httpfilter
                     ${karchive_SOURCE_DIR}/src
                     ${karchive_BINARY_DIR}/src
)
include_directories(${ZLIB_INCLUDE_DIR})

########### next target ###############

set(kmultipart_PART_SRCS kmultipart.cpp ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc )


kde4_add_plugin(kmultipart WITH_PREFIX ${kmultipart_PART_SRCS})

target_link_libraries(kmultipart  ${KDE4_KPARTS_LIBS} ${ZLIB_LIBRARY} karchive ${kde4support_LIBRARY} ${KWidgetsAddons_LIBRARIES})

install(TARGETS kmultipart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kmultipart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




