include(ECMMarkAsTest)

set(krosstest_SRCS
   testobject.cpp
   main.cpp )


add_executable(krosstest ${krosstest_SRCS})
ecm_mark_as_test(${krosstest})
target_link_libraries(krosstest KrossCore ${KCoreAddons_LIBRARIES} KConfigCore Qt5::Widgets)

