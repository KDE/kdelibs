add_subdirectory(lib)
if (NOT MSVC)
  add_subdirectory(plugins)
endif()
