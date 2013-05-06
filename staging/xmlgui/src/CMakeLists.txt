find_package(X11)

if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()

configure_file(config-xmlgui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xmlgui.h )

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(xmlgui_SRCS
  kglobalaccel.cpp
  kglobalshortcutinfo.cpp
  kglobalshortcutinfo_dbus.cpp
)

set(kglobalaccel_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(xmlgui_SRCS ${kglobalaccel_xml} kglobalaccel_interface )
install(FILES ${kglobalaccel_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(kglobalaccel_component_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(xmlgui_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface )
install(FILES ${kglobalaccel_component_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


add_library(XmlGui ${xmlgui_SRCS})
generate_export_header(XmlGui)

target_link_libraries(XmlGui LINK_PUBLIC Qt5::Widgets Qt5::DBus Qt5::X11Extras)

set_target_properties(XmlGui PROPERTIES VERSION   ${XMLGUI_VERSION_STRING}
                                        SOVERSION ${XMLGUI_SOVERSION}
)

install(TARGETS XmlGui EXPORT XmlGuiTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kglobalaccel.h
  kglobalshortcutinfo.h
  kglobalshortcutinfo_p.h
  ${CMAKE_CURRENT_BINARY_DIR}/xmlgui_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
