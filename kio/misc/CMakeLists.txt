
# for strange kio/foo.h includes
include_directories( BEFORE ${CMAKE_SOURCE_DIR}/kio )

add_subdirectory( ksendbugmail )
if(NOT WIN32)
  # arpa/nameser.h
  add_subdirectory( kpac )
endif(NOT WIN32)

