
find_package(Qt5Widgets 5.2.0 REQUIRED NO_MODULE)
find_package(Qt5Xml 5.2.0 REQUIRED NO_MODULE)

set(libkconfiggui_SRCS
   kconfiggui.cpp
   kconfiggroupgui.cpp
   kconfigloader.cpp
   kconfigskeleton.cpp
   kstandardshortcut.cpp
   kwindowconfig.cpp
)

add_library(KF5ConfigGui ${libkconfiggui_SRCS})
generate_export_header(KF5ConfigGui BASE_NAME KConfigGui)
add_library(KF5::ConfigGui ALIAS KF5ConfigGui)

target_link_libraries(KF5ConfigGui PUBLIC Qt5::Gui Qt5::Xml KF5::ConfigCore)

if(IS_ABSOLUTE "${INCLUDE_INSTALL_DIR}")
  target_include_directories(KF5ConfigGui INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>" )
else()
  target_include_directories(KF5ConfigGui INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}>" )
endif()

set_target_properties(KF5ConfigGui PROPERTIES VERSION   ${KCONFIG_VERSION_STRING}
                                              SOVERSION ${KCONFIG_SOVERSION}
                                              EXPORT_NAME ConfigGui
)

install(TARGETS KF5ConfigGui EXPORT KF5ConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfiggui_export.h
   kconfiggui.h
   kconfigloader.h
   kconfigskeleton.h
   kstandardshortcut.h
   kwindowconfig.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
