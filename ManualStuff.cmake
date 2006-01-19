
if (UNIX)
   link_directories(/usr/local/lib)
   include_directories(/usr/local/include)
endif (UNIX)

exec_program(cmake ARGS -E copy ${CMAKE_SOURCE_DIR}/admin/install-sh ${CMAKE_SOURCE_DIR})
exec_program(cmake ARGS -E copy ${CMAKE_SOURCE_DIR}/admin/config.sub ${CMAKE_SOURCE_DIR})
exec_program(cmake ARGS -E copy ${CMAKE_SOURCE_DIR}/admin/config.guess ${CMAKE_SOURCE_DIR})

option(INCLUDE_SVGICONS "Include svg icons")

include_directories(
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kio/kio 
  ${CMAKE_SOURCE_DIR}/libltdl
  ${CMAKE_SOURCE_DIR}/kio/kfile 
  ${CMAKE_SOURCE_DIR}/interfaces
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kssl
  ${CMAKE_SOURCE_DIR}/kdefx
)
