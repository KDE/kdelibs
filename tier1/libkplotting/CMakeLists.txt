project(kplotting)

include(KDE4Defaults)

include_directories(
  ${QT_INCLUDES}
)

add_subdirectory(src)
add_subdirectory(examples)

