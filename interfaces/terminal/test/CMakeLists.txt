set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)


########### next target ###############



set(test_SRCS main.cc )
add_executable(test ${test_SRCS})
ecm_mark_as_test(test)
target_link_libraries(test  ${KDE4_KDECORE_LIBS} kdeui kio )

########### install files ###############





