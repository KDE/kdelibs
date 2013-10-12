
include_directories( ${KHTML_SOURCE_DIR}
                     ${KHTML_BINARY_DIR}
                     ${CMAKE_SOURCE_DIR}/kio
                     ${CMAKE_SOURCE_DIR}/kio/httpfilter
                     ${KArchive_INCLUDE_DIRS}
)
include_directories(${ZLIB_INCLUDE_DIR})

########### next target ###############

set(kmultipart_PART_SRCS kmultipart.cpp ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc )
add_library(kmultipart MODULE ${kmultipart_PART_SRCS})
set_target_properties(kmultipart PROPERTIES PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")

target_link_libraries(kmultipart KParts ${ZLIB_LIBRARY} ${KArchive_LIBRARIES} ${KDE4Support_LIBRARIES} KF5::KWidgetsAddons XmlGui KF5::KI18n KTextWidgets)

install(TARGETS kmultipart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kmultipart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




