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
add_library(KF5::KCompletion ALIAS KCompletion)

target_link_libraries(KCompletion LINK_PUBLIC Qt5::Widgets
                                  LINK_PRIVATE KConfigCore      # KConfigGroup, used in many places
                                               KConfigWidgets   # KStandardAction, KStandardShortcut, KCursor
)

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
