
if(KDE4_BUILD_TESTS)

########### next target ###############

set(kcookiejartest_SRCS kcookiejartest.cpp )

kde4_automoc(${kcookiejartest_SRCS})

kde4_add_executable(kcookiejartest NOGUI ${kcookiejartest_SRCS})

# TODO port kcookiejar.h away from Q3PtrList
target_link_libraries(kcookiejartest  ${KDE4_KIO_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

endif(KDE4_BUILD_TESTS)

########### install files ###############





