add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(kde4attic_SRCS
  kanimatedbutton.cpp
  kcharselect.cpp
  kcharselectdata.cpp
  kdatecombobox.cpp
  kdatetimeedit.cpp
  kdatetimewidget.cpp
  kdatewidget.cpp
  kmultitabbar.cpp
  ktimecombobox.cpp
  ktimezonewidget.cpp
)

set(kde4attic_UI
  kdatetimeedit.ui
)

kde4_add_ui_files(kde4attic_SRCS ${kde4attic_UI} )

add_library(KDE4Attic ${kde4attic_SRCS})
generate_export_header(KDE4Attic)

target_link_libraries(KDE4Attic LINK_PUBLIC Qt5::Widgets kde4support kdeui kdecore)

set_target_properties(KDE4Attic PROPERTIES VERSION   ${KDE4ATTIC_VERSION_STRING}
                                           SOVERSION ${KDE4ATTIC_SOVERSION}
)

install(TARGETS KDE4Attic EXPORT KDE4AtticTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kanimatedbutton.h
  kcharselect.h
  kdatecombobox.h
  kdatetimeedit.h
  kdatetimewidget.h
  kdatewidget.h
  kmultitabbar.h
  ktimecombobox.h
  ktimezonewidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/kde4attic_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES kcharselect-data  DESTINATION  ${DATA_INSTALL_DIR}/kcharselect)
