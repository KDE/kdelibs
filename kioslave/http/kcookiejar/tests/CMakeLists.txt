
if(KDE4_BUILD_TESTS)

########### next target ###############

set(kcookiejartest_SRCS kcookiejartest.cpp )

kde4_automoc(${kcookiejartest_SRCS})

kde4_add_executable(kcookiejartest NOGUI ${kcookiejartest_SRCS})

target_link_libraries(kcookiejartest  ${KDE4_KDECORE_LIBS} kio )

endif(KDE4_BUILD_TESTS)

########### install files ###############





