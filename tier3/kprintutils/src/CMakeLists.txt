remove_definitions(-DQT_NO_CAST_FROM_ASCII)

configure_file(config-kprintutils.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kprintutils.h)

set(kprintutils_LIB_SRCS
    kprintpreview.cpp
)

set(kprintutils_LIB_SRCS
  ${kprintutils_LIB_SRCS}
  kdeprintdialog.cpp
)

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
endif ()

qt5_wrap_ui(kprintutils_LIB_SRCS ${kprintutils_UI})

add_library(KF5PrintUtils ${kprintutils_LIB_SRCS})
generate_export_header(KF5PrintUtils BASE_NAME KPrintUtils)
add_library(KF5::PrintUtils ALIAS KF5PrintUtils)

target_link_libraries(KF5PrintUtils
  PUBLIC
    Qt5::Widgets
    Qt5::PrintSupport
  PRIVATE
    KF5::Parts
    KF5::I18n
)
set_target_properties(KF5PrintUtils PROPERTIES
                      VERSION ${KPRINTUTILS_VERSION_STRING}
                      SOVERSION ${KPRINTUTILS_SOVERSION}
                      EXPORT_NAME PrintUtils
)

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kprintutils_export.h
 kdeprintdialog.h
 kprintpreview.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS KF5PrintUtils EXPORT KF5PrintUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


