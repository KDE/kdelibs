project(bihash_test)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4)

include(${QT_USE_FILE})

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/../../itemviews
  ${PROJECT_BINARY_DIR}
)

set(functionalitytest_srcs
  functionalitytest.cpp
)

add_executable(bihash_functionalitytest ${functionalitytest_srcs})

target_link_libraries(bihash_functionalitytest
  ${QT_QTCORE_LIBRARIES}
)

set(benchmarks_srcs
  benchmarks.cpp
)
set(benchmarks_srcs2
  benchmarks.cpp
)

# set(moc_output_file "${CMAKE_CURRENT_BINARY_DIR}/benchmarks.moc")

# qt4_generate_moc(benchmarks.cpp ${moc_output_file})
add_executable(bihash_benchmarks_exec ${benchmarks_srcs} )
add_test(bihash_benchmarks bihash_benchmarks_exec)

target_link_libraries(bihash_benchmarks_exec
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTTEST_LIBRARIES}
)
