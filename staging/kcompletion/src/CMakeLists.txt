find_package(X11)

if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)


configure_file(config-kcompletion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcompletion.h )

set(kcompletion_SRCS
  kcombobox.cpp
  kcompletion.cpp
  kcompletionbase.cpp
  kcompletionbox.cpp
  klineedit.cpp
  khistorycombobox.cpp
  kpixmapprovider.cpp
  kzoneallocator.cpp
)

add_library(KCompletion ${kcompletion_SRCS})
generate_export_header(KCompletion)

target_link_libraries(KCompletion LINK_PUBLIC Qt5::Widgets)
target_link_libraries(KCompletion LINK_PRIVATE KNotifications KCoreAddons KConfigCore KWidgetsAddons KIconThemes KWidgets KI18n)

set_target_properties(KCompletion PROPERTIES VERSION   ${KCOMPLETION_VERSION_STRING}
                                             SOVERSION ${KCOMPLETION_SOVERSION}
)

install(TARGETS KCompletion EXPORT KCompletionTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kcombobox.h
  kcompletion.h
  kcompletionbox.h
  klineedit.h
  khistorycombobox.h
  kpixmapprovider.h
  ksortablelist.h
  ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
