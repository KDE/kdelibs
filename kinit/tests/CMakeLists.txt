set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KIO_INCLUDES} )

set(klaunchertest_SRCS klaunchertest.cpp )


kde4_add_executable(klaunchertest TEST ${klaunchertest_SRCS})

target_link_libraries(klaunchertest  ${KDE4_KDECORE_LIBS} kio )

########### install files ###############





