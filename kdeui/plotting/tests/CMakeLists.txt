if(KDE4_BUILD_TESTS)
include_directories( ${CMAKE_SOURCE_DIR}/libkdeedu/kdeeduplot  )


########### next target ###############

set(testplot_SRCS testplot_widget.cpp testplot_main.cpp )

kde4_automoc(${testplot_SRCS})

kde4_add_executable(testplot ${testplot_SRCS})

target_link_libraries(testplot  ${KDE4_KDECORE_LIBS} kdeeduplot m )

endif(KDE4_BUILD_TESTS)

