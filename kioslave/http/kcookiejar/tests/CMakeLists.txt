set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(kcookiejartest_SRCS kcookiejartest.cpp )

kde4_add_unit_test(kcookiejartest NOGUI ${kcookiejartest_SRCS})

target_link_libraries(kcookiejartest  ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

########### install files ###############





