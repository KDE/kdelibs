project(bihash_test)

cmake_minimum_required(VERSION 2.8.10.20130309)

find_package(Qt4)

include(${QT_USE_FILE})

set(functionalitytest_srcs
  functionalitytest.cpp
)

add_executable(bihash_functionalitytest ${functionalitytest_srcs})

target_link_libraries(bihash_functionalitytest
  Qt5::Core
)

set(benchmarks_srcs
  benchmarks.cpp
)
set(benchmarks_srcs2
  benchmarks.cpp
)

# set(moc_output_file "${CMAKE_CURRENT_BINARY_DIR}/benchmarks.moc")

# qt5_generate_moc(benchmarks.cpp ${moc_output_file})
add_executable(bihash_benchmarks_exec ${benchmarks_srcs} )
add_test(bihash_benchmarks bihash_benchmarks_exec)

target_link_libraries(bihash_benchmarks_exec
  Qt5::Widgets
  ${QT_QTTEST_LIBRARIES}
)
