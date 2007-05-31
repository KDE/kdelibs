project(kcert)

add_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

#include_directories( ${CMAKE_SOURCE_DIR}/kio/kssl ${CMAKE_BINARY_DIR}/kio/kssl )
include_directories( ${KDE4_KIO_INCLUDES} )

if(OPENSSL_FOUND)
  include_directories( ${OPENSSL_INCLUDE_DIR} )
endif(OPENSSL_FOUND)

set(kcertpart_PART_SRCS kcertpart.cc )

kde4_automoc(${kcertpart_PART_SRCS})

kde4_add_plugin(kcertpart WITH_PREFIX ${kcertpart_PART_SRCS})

# Needs Qt3Support for Q3ListViewItem
target_link_libraries(kcertpart  ${KDE4_KPARTS_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

install(TARGETS kcertpart  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kcertpart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kcertpart.rc  DESTINATION  ${DATA_INSTALL_DIR}/kcertpart )

