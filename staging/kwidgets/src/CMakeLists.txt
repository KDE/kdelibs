
set(kwidgets_LIB_SRCS
  utils/kglobalsettings.cpp
)

add_library(kwidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})
generate_export_header(kwidgets)

target_link_libraries(kwidgets
  ${X11_LIBRARIES} ${X11_Xcursor_LIB}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  kdeguiaddons
)

set_target_properties(kwidgets PROPERTIES VERSION ${ECM_VERSION_STRING}
                                        SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kwidgets EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
  utils/kglobalsettings.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
