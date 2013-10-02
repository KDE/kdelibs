remove_definitions(-DQT_NO_CAST_FROM_ASCII)

configure_file(config-kprintutils.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kprintutils.h)

set(kprintutils_LIB_SRCS
    kprintpreview.cpp
)

if(NOT WINCE)
set(kprintutils_LIB_SRCS
  ${kprintutils_LIB_SRCS}
  kdeprintdialog.cpp
)
endif(NOT WINCE)

if (X11_FOUND)
set(kprintutils_LIB_SRCS
  ${kprintutils_LIB_SRCS}
  kcupsoptionswidget_p.cpp
  kcupsoptionspageswidget_p.cpp
)

set(kprintutils_UI
  ${kprintutils_UI}
  kcupsoptionspageswidget.ui
)
endif (X11_FOUND)

qt5_wrap_ui(kprintutils_LIB_SRCS ${kprintutils_UI})

add_library(KPrintUtils ${kprintutils_LIB_SRCS})
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
 kdeprintdialog.h
 kprintpreview.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS KPrintUtils EXPORT KPrintUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


