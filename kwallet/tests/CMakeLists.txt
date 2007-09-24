set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDEUI_INCLUDES} ) # for kwallet.h

########### next target ###############

set(kwalletasync_SRCS kwalletasync.cpp kwallettest.cpp )


kde4_add_executable(kwalletasync TEST ${kwalletasync_SRCS})

target_link_libraries(kwalletasync ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(kwalletsync_SRCS kwalletsync.cpp )


kde4_add_executable(kwalletsync TEST ${kwalletsync_SRCS})

target_link_libraries(kwalletsync  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(kwalletboth_SRCS kwalletboth.cpp kwallettest.cpp )


kde4_add_executable(kwalletboth TEST ${kwalletboth_SRCS})

target_link_libraries(kwalletboth  ${KDE4_KDEUI_LIBS} )

########### install files ###############

