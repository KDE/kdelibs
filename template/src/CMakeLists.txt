set(foobar_SRCS
  myclass.cpp
)

add_library(FooBar ${foobar_SRCS})
generate_export_header(FooBar)
add_library(KF5::FooBar ALIAS FooBar)

target_link_libraries(FooBar
  LINK_PUBLIC
    Qt5::Core
  # LINK_PRIVATE
  #  Other private libs
)

set_target_properties(FooBar
  PROPERTIES VERSION ${FOOBAR_VERSION_STRING}
  SOVERSION ${FOOBAR_SOVERSION}
)

# Install files
install(TARGETS FooBar
  EXPORT FooBarTargets
  ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  myclass.h
  ${CMAKE_CURRENT_BINARY_DIR}/foobar_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
  COMPONENT Devel
)
