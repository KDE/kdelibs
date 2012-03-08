add_subdirectory(kauth)
add_subdirectory(sonnet)
add_subdirectory(kcoreaddons)
add_subdirectory(karchive)
add_subdirectory(kde4support)
add_subdirectory(kcolorwidgets)
add_subdirectory(kdeguiaddons)
add_subdirectory(kwidgets)
if(NOT QT5_BUILD)
  add_subdirectory(kwindowsystem)
  # TODO: Remove this:
  set(kwindowsystem_LIBRARY kwindowsystem)
endif()

