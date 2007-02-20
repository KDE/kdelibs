
if(KDE4_BUILD_TESTS)

   # Tests don't need to go into toplevel/bin, they are fine in the current dir.
   set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

   ########### next target ###############

   set(helpviewer_SRCS
      helpwindow.cpp
      main.cpp
      richpage.cpp
      )

   kde4_automoc(${helpviewer_SRCS})

   kde4_add_executable(helpviewer ${helpviewer_SRCS})

   target_link_libraries(helpviewer  ${KDE4_KDECORE_LIBS} kdeprint )


   ########### next target ###############

   set(drawdemo_SRCS drawdemo.cpp )

   kde4_automoc(${drawdemo_SRCS})

   kde4_add_executable(drawdemo ${drawdemo_SRCS})

   target_link_libraries(drawdemo  ${KDE4_KDECORE_LIBS} kdeprint )

endif(KDE4_BUILD_TESTS)

########### install files ###############





