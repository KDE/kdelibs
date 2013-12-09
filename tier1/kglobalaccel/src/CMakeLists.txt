configure_file(config-kglobalaccel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kglobalaccel.h )

set(kglobalaccel_SRCS
  kglobalaccel.cpp
  kglobalshortcutinfo.cpp
  kglobalshortcutinfo_dbus.cpp
)

set(kglobalaccel_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(kglobalaccel_SRCS ${kglobalaccel_xml} kglobalaccel_interface )
install(FILES ${kglobalaccel_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(kglobalaccel_component_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(kglobalaccel_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface )
install(FILES ${kglobalaccel_component_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_library(KF5GlobalAccel ${kglobalaccel_SRCS})
generate_export_header(KF5GlobalAccel BASE_NAME KGlobalAccel)
add_library(KF5::GlobalAccel ALIAS KF5GlobalAccel)

target_link_libraries(KF5GlobalAccel
PUBLIC
    Qt5::DBus
    Qt5::Widgets
PRIVATE
    Qt5::X11Extras
)

set_target_properties(KF5GlobalAccel PROPERTIES VERSION   ${KGLOBALACCEL_VERSION_STRING}
                                                SOVERSION ${KGLOBALACCEL_SOVERSION}
                                                EXPORT_NAME "GlobalAccel"
)

install(TARGETS KF5GlobalAccel EXPORT KF5GlobalAccelTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kglobalaccel.h
  kglobalshortcutinfo.h
  ${CMAKE_CURRENT_BINARY_DIR}/kglobalaccel_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
