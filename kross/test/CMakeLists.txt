set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

set(krosstest_SRCS
   testobject.cpp
   main.cpp )


add_executable(krosstest ${krosstest_SRCS})
ecm_mark_as_test(${krosstest})
target_link_libraries(krosstest ${KDE4_KDEUI_LIBS} krosscore ${KCoreAddons_LIBRARIES} )

