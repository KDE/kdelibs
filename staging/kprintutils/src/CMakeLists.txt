remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kprintutils_LIB_SRCS
    kprintpreview.cpp
)

add_library(KPrintUtils ${LIBRARY_TYPE} ${kprintutils_LIB_SRCS})
generate_export_header(KPrintUtils)

target_link_libraries(KPrintUtils
  LINK_PUBLIC
    Qt5::Widgets
    Qt5::PrintSupport
  LINK_PRIVATE
    KParts
    KCoreAddons
    KI18n
)
set_target_properties(KPrintUtils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kprintutils_export.h
 kprintpreview.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS KPrintUtils EXPORT KPrintUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


