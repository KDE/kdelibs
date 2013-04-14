set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

kde4_add_unit_test(kptyprocesstest kptyprocesstest.cpp)
target_link_libraries(kptyprocesstest ${KDE4_KPTY_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )
