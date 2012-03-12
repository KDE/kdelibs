
set(kwidgets_LIB_SRCS
  utils/kglobalsettings.cpp
  icons/kicon.cpp
  icons/kiconeffect.cpp
  icons/kiconengine.cpp
  icons/kiconloader.cpp
  icons/kicontheme.cpp
  widgets/khbox.cpp
  widgets/kvbox.cpp
  widgets/kseparator.cpp
  widgets/kurllabel.cpp
  utils/kstandardguiitem.cpp
  utils/kguiitem.cpp
)

add_library(kwidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})
generate_export_header(kwidgets)

target_link_libraries(kwidgets
  ${X11_LIBRARIES} ${X11_Xcursor_LIB}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  kdeguiaddons
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
  icons/kicon.h
  icons/kiconeffect.h
  widgets/khbox.h
  widgets/kvbox.h
  widgets/kseparator.h
  widgets/kurllabel.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
