
if(KDE4_BUILD_TESTS)

   # Tests don't need to go into toplevel/bin, they are fine in the current dir.
   set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

   include_directories( ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/khtml )
   
   
   ########### next target ###############
   
   set(testserver_SRCS testkjavaappletserver.cpp )
   
   kde4_automoc(${testserver_SRCS})
   
   kde4_add_executable(testserver ${testserver_SRCS})
   
   target_link_libraries(testserver  ${KDE4_KDECORE_LIBS} khtml)

endif(KDE4_BUILD_TESTS)


