
if(KDE4_BUILD_TESTS)

   # Tests don't need to go into toplevel/bin, they are fine in the current dir.
   set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

   include_directories( ${KDE4_KIO_INCLUDES} )
   
   set(klaunchertest_SRCS klaunchertest.cpp )
   
   kde4_automoc(${klaunchertest_SRCS})
   
   kde4_add_executable(klaunchertest ${klaunchertest_SRCS})
   
   target_link_libraries(klaunchertest  ${KDE4_KDECORE_LIBS} kio )

endif(KDE4_BUILD_TESTS)

########### install files ###############





