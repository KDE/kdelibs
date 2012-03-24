add_subdirectory(kauth)
add_subdirectory(sonnet)
add_subdirectory(karchive)
add_subdirectory(kconfig)
add_subdirectory(kde4support)
if(NOT QT5_BUILD)
  add_subdirectory(kwindowsystem)
  # TODO: Remove this:
  set(kwindowsystem_LIBRARY kwindowsystem)
endif()

