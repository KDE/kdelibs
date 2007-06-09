if(KDE4_BUILD_TESTS)


########### next target ###############

set(kuiservertest_SRCS kuiservertest.cpp)

kde4_automoc(${kuiservertest_SRCS})

kde4_add_executable(kuiservertest NOGUI RUN_UNINSTALLED ${kuiservertest_SRCS})

target_link_libraries(kuiservertest ${KDE4_KIO_LIBS})


endif(KDE4_BUILD_TESTS)
