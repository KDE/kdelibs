project(kmetadata)

add_subdirectory(rcgen)
add_subdirectory(kmetadata)
add_subdirectory(ontologies)

if(KDE4_BUILD_TESTS)
  add_subdirectory(tests)
endif(KDE4_BUILD_TESTS)
