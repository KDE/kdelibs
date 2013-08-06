project(tetest)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

include(ECMMarkAsTest)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

set(hello_SRCS main.cc )
kde4_add_executable(tetest ${hello_SRCS})
target_link_libraries(tetest ${KDE4_KPARTS_LIBS})


set(test_SRCS main.cc )
add_executable(test ${test_SRCS})
ecm_mark_as_test(test)
target_link_libraries(test kio)

########### install files ###############
install(TARGETS tetest DESTINATION ${BIN_INSTALL_DIR})
