add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(kde4attic_SRCS
  kanimatedbutton.cpp
)

add_library(KDE4Attic ${kde4attic_SRCS})
generate_export_header(KDE4Attic)

target_link_libraries(KDE4Attic LINK_PUBLIC Qt5::Widgets kde4support kdeui kdecore)

set_target_properties(KDE4Attic PROPERTIES VERSION   ${KDE4ATTIC_VERSION_STRING}
                                           SOVERSION ${KDE4ATTIC_SOVERSION}
)

install(TARGETS KDE4Attic EXPORT KDE4AtticTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kanimatedbutton.h
  ${CMAKE_CURRENT_BINARY_DIR}/kde4attic_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
