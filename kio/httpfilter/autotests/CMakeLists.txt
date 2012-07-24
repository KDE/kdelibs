remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

include(ECMMarkAsTest)

########### httpfiltertest ###############

# compile httpfilter.cpp into the test since it's not part of kdelibs
# (only part of kio_http and kmultipart)

add_executable(httpfiltertest httpfiltertest.cpp ../httpfilter.cc)
target_link_libraries(httpfiltertest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY} karchive ${inqt5_LIBRARY} ${ZLIB_LIBRARIES})

add_test(kiohttp-httpfiltertest httpfiltertest)
