
if(KDE4_BUILD_TESTS)


########### next target ###############

set(kwalletasync_SRCS kwalletasync.cpp kwallettest.cpp )

kde4_automoc(${kwalletasync_SRCS})

kde4_add_executable(kwalletasync NOGUI ${kwalletasync_SRCS})

target_link_libraries(kwalletasync ${KDE4_KDECORE_LIBS} kwalletclient )


########### next target ###############

set(kwalletsync_SRCS kwalletsync.cpp )

kde4_automoc(${kwalletsync_SRCS})

kde4_add_executable(kwalletsync NOGUI ${kwalletsync_SRCS})

target_link_libraries(kwalletsync  ${KDE4_KDECORE_LIBS} kwalletclient )


########### next target ###############

set(kwalletboth_SRCS kwalletboth.cpp kwallettest.cpp )

kde4_automoc(${kwalletboth_SRCS})

kde4_add_executable(kwalletboth NOGUI ${kwalletboth_SRCS})

target_link_libraries(kwalletboth  ${KDE4_KDECORE_LIBS} kwalletclient )

endif(KDE4_BUILD_TESTS)

########### install files ###############





