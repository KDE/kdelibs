
find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)

configure_file(config-kwidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwidgets.h )

set(kwidgets_LIB_SRCS
  utils/kglobalsettings.cpp
  icons/kiconeffect.cpp
  icons/kiconengine.cpp
  icons/kiconloader.cpp
  icons/kicontheme.cpp
  widgets/kurllabel.cpp # Not good enough quality. Needs to use QUrl instead of QString and should not inherit QLabel, but hold it as a member instead.
  widgets/kpushbutton.cpp
  utils/kstandardguiitem.cpp
  utils/kguiitem.cpp
  dialogs/kdialog.cpp
)

add_library(kwidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})
generate_export_header(kwidgets)

target_link_libraries(kwidgets
  ${X11_LIBRARIES} ${X11_Xcursor_LIB}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBS} # KGlobal, KLocale, KLocalizedString
  ${inqt5_LIBRARY}
  kconfigcore
  kconfiggui
  kcoreaddons
  kdeguiaddons
  kwidgetsaddons
  kauth
  ${kwindowsystem_LIBRARY}
)

set_target_properties(kwidgets PROPERTIES VERSION ${ECM_VERSION_STRING}
                                        SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kwidgets EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
  utils/kglobalsettings.h
  utils/kstandardguiitem.h
  utils/kguiitem.h
  icons/kicontheme.h
  icons/kiconloader.h
  icons/kiconeffect.h
  widgets/kpushbutton.h
  widgets/kurllabel.h
  dialogs/kdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
