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

add_library(KF5Completion ${kcompletion_SRCS})
generate_export_header(KF5Completion BASE_NAME KCompletion)
add_library(KF5::Completion ALIAS KF5Completion)

target_link_libraries(KF5Completion PUBLIC Qt5::Widgets
                                    PRIVATE KF5::ConfigCore      # KConfigGroup, used in many places
                                            KF5::ConfigGui       # KStandardShortcut
                                            KF5::WidgetsAddons   # KCursor
)

set_target_properties(KF5Completion PROPERTIES VERSION   ${KCOMPLETION_VERSION_STRING}
                                               SOVERSION ${KCOMPLETION_SOVERSION}
                                               EXPORT_NAME Completion
)

install(TARGETS KF5Completion EXPORT KF5CompletionTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
