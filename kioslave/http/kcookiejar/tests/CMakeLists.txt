set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

find_package(Qt5Test REQUIRED)

########### next target ###############

set(kcookiejartest_SRCS kcookiejartest.cpp )

kde4_add_unit_test(kcookiejartest NOGUI ${kcookiejartest_SRCS})

target_link_libraries(kcookiejartest  ${KDE4_KIO_LIBS} Qt5::Test KF5::KCoreAddons)

########### install files ###############





