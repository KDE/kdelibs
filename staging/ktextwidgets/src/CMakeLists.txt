set(ktextwidgets_LIB_SRCS
  findreplace/kfind.cpp
  findreplace/kfinddialog.cpp
  findreplace/kreplace.cpp
  findreplace/kreplacedialog.cpp
  widgets/ktextedit.cpp
)

add_library(KTextWidgets ${LIBRARY_TYPE} ${ktextwidgets_LIB_SRCS})
generate_export_header(KTextWidgets)

set(ktextwidgets_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
  ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)
target_include_directories(KTextWidgets PUBLIC "$<BUILD_INTERFACE:${ktextwidgets_INCLUDES}>"
)

target_link_libraries(KTextWidgets
  Qt5::Widgets
  Qt5::DBus
  SonnetCore
  SonnetUi
  ki18n
  kwidgets
  KWidgetsAddons
  kservice
  KConfigCore
  KWindowSystem
  KIconThemes
  KCompletion
)

set_target_properties(KTextWidgets PROPERTIES VERSION ${KTEXTWIDGETS_VERSION_STRING}
                                   SOVERSION ${KTEXTWIDGETS_SOVERSION}
)

install(TARGETS KTextWidgets EXPORT KTextWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  findreplace/kfind.h
  findreplace/kfinddialog.h
  findreplace/kreplace.h
  findreplace/kreplacedialog.h
  widgets/ktextedit.h
  ${CMAKE_CURRENT_BINARY_DIR}/ktextwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
