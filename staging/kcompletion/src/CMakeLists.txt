add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(kcompletion_SRCS
  kpixmapprovider.cpp
)

add_library(KCompletion ${kcompletion_SRCS})
generate_export_header(KCompletion)

target_link_libraries(KCompletion LINK_PUBLIC Qt5::Widgets)

set_target_properties(KCompletion PROPERTIES VERSION   ${KCOMPLETION_VERSION_STRING}
                                             SOVERSION ${KCOMPLETION_SOVERSION}
)

install(TARGETS KCompletion EXPORT KCompletionTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kpixmapprovider.h
  ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
