set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

set(helpviewer_SRCS
   helpwindow.cpp
   main.cpp
   richpage.cpp
   )


kde4_add_executable(helpviewer TEST ${helpviewer_SRCS})

target_link_libraries(helpviewer  ${KDE4_KDECORE_LIBS} kdeprint ${QT_QT3SUPPORT_LIBRARY})


########### next target ###############

set(drawdemo_SRCS drawdemo.cpp )


kde4_add_executable(drawdemo TEST ${drawdemo_SRCS})

target_link_libraries(drawdemo  ${KDE4_KDECORE_LIBS} kdeprint )

########### install files ###############





