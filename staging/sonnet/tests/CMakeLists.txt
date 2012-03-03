set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
  ${CMAKE_CURRENT_BINARY_DIR}/../src/core

  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/config
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/io
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/util
  ${CMAKE_CURRENT_BINARY_DIR}/../../../kdecore
  ${CMAKE_CURRENT_BINARY_DIR}/../../../staging/kcoreaddons/src
)
########### next target ###############

# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)

add_executable(test_suggest ${test_suggest_SRCS})

target_link_libraries(test_suggest ${KDE4_KDECORE_LIBS} sonnetcore)

