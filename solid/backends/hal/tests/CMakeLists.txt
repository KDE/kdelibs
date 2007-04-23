

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )


########### next target ###############

if(KDE4_BUILD_TESTS)
set(halbasictest_SRCS halbasictest.cpp )

kde4_automoc(${halbasictest_SRCS})


kde4_add_executable(halbasictest RUN_UNINSTALLED ${halbasictest_SRCS})

target_link_libraries(halbasictest solid_hal_discovery_static ${QT_QTTEST_LIBRARY} )

add_test(halbasictest ${EXECUTABLE_OUTPUT_PATH}/halbasictest)

endif(KDE4_BUILD_TESTS)

########### install files ###############



