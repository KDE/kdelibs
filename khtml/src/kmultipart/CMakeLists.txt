
include_directories( ${KHTML_SOURCE_DIR}
                     ${KHTML_BINARY_DIR}
                     ${CMAKE_SOURCE_DIR}/kio
                     ${CMAKE_SOURCE_DIR}/kio/httpfilter
)
include_directories(${ZLIB_INCLUDE_DIR})

########### next target ###############

set(kmultipart_PART_SRCS kmultipart.cpp ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc )
add_library(kmultipart MODULE ${kmultipart_PART_SRCS})
set_target_properties(kmultipart PROPERTIES PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")

target_link_libraries(kmultipart
                        ${ZLIB_LIBRARY}
                        KF5::KParts
                        KF5::KArchive
                        KF5::KDE4Support
                        KF5::KWidgetsAddons
                        KF5::XmlGui
                        KF5::KI18n
                        KF5::KTextWidgets)

install(TARGETS kmultipart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kmultipart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




