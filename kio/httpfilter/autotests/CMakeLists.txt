set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${KArchive_INCLUDE_DIRS}
)

include(ECMMarkAsTest)

########### httpfiltertest ###############

# compile httpfilter.cpp into the test since it's not part of kdelibs
# (only part of kio_http and kmultipart)

add_executable(httpfiltertest httpfiltertest.cpp ../httpfilter.cc)
target_link_libraries(httpfiltertest ${QT_QTTEST_LIBRARY} ${KArchive_LIBRARIES} ${ZLIB_LIBRARIES})

add_test(kiohttp-httpfiltertest httpfiltertest)
