include_directories(${KDE4_KIO_INCLUDES})

set(krosstest_SRCS 
   testobject.cpp 
   main.cpp )

kde4_automoc(${krosstest_SRCS})

kde4_add_test_executable(krosstest ${krosstest_SRCS})

target_link_libraries(krosstest ${KDE4_KDEUI_LIBS} krosscore )

