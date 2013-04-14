
find_package(X11)
if(X11_FOUND)
  macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)


# QUrl
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)


configure_file(config-kwidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwidgets.h )

set(kwidgets_LIB_SRCS
  dialogs/kmessagebox.cpp
  icons/kiconeffect.cpp
  icons/kiconengine.cpp
  icons/kiconloader.cpp # cannot move down to kguiaddons: depends on KSharedDataCache (kcoreaddons) and localizedFilePath (ki18n)
  icons/kicontheme.cpp
  utils/kglobalsettings.cpp
  utils/kguiitem.cpp
  utils/khelpclient.cpp
  utils/kstandardguiitem.cpp
  widgets/kdragwidgetdecorator.cpp
  widgets/kurllabel.cpp # Not good enough quality. Needs to use QUrl instead of QString and should not inherit QLabel, but hold it as a member instead.
)

add_library(kwidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})
generate_export_header(kwidgets)

set(kwidgets_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/icons
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)
target_include_directories(kwidgets PUBLIC "$<BUILD_INTERFACE:${kwidgets_INCLUDES}>" "$<TARGET_PROPERTY:kdeui,INTERFACE_INCLUDE_DIRECTORIES>"
)

target_link_libraries(kwidgets
  ${X11_LIBRARIES} ${X11_Xcursor_LIB}
  Qt5::Widgets
  Qt5::Svg
  Qt5::DBus
  ${KDE4_KDECORE_LIBS} # KGlobal, KLocale, KLocalizedString

  ${KConfig_CORE_LIBRARY}
  ${KConfig_GUI_LIBRARY}
  ${KCoreAddons_LIBRARIES}
  kguiaddons
  ${KWidgetsAddons_LIBRARIES}
  ${KAuth_LIBRARIES}
  ${KWindowSystem_LIBRARIES}
  kservice
)

if(X11_FOUND)
  find_package(Qt5Transitional REQUIRED MODULE COMPONENTS X11Extras)
  target_link_libraries(kwidgets Qt5::X11Extras)
endif()

set_target_properties(kwidgets PROPERTIES VERSION ${KWIDGETS_VERSION_STRING}
                                        SOVERSION ${KWIDGETS_SOVERSION}
)

# TODO : export into KWidgetsTargets instead of kdelibsLibraryTargets
install(TARGETS kwidgets EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  dialogs/kmessagebox.h
  icons/kiconeffect.h
  icons/kiconloader.h
  icons/kicontheme.h
  utils/kglobalsettings.h
  utils/kguiitem.h
  utils/khelpclient.h
  utils/kstandardguiitem.h
  widgets/kdragwidgetdecorator.h
  widgets/kurllabel.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
