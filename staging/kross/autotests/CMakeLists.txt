include(ECMMarkAsTest)

set(krosstest_SRCS
   testobject.cpp
   main.cpp )


add_executable(krosstest ${krosstest_SRCS})
target_link_libraries(krosstest KrossCore KF5::KCoreAddons KConfigCore Qt5::Widgets)

add_test(kross-guiform-py krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testguiform.py)
add_test(kross-guiform-rb krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testguiform.rb)
add_test(kross-guiqt-py krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testguiqt.py)
add_test(kross-guiqt-rb krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testguiqt.rb)
add_test(kross-guitk-py krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testguitk.py)
add_test(kross-test-js krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testkross.js)
add_test(kross-test-py krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testkross.py)
add_test(kross-unittest-es krosstest ${CMAKE_CURRENT_SOURCE_DIR}/unittest.es)
add_test(kross-unittest-js krosstest ${CMAKE_CURRENT_SOURCE_DIR}/unittest.js)
add_test(kross-unittest-py krosstest ${CMAKE_CURRENT_SOURCE_DIR}/unittest.py)
add_test(kross-unittest-rb krosstest ${CMAKE_CURRENT_SOURCE_DIR}/unittest.rb)

# UI files have no interpreter
# add_test(kross-guiform-ui krosstest ${CMAKE_CURRENT_SOURCE_DIR}/testguiform.ui)
