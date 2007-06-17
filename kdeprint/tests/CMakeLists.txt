

########### next target ###############

set(helpviewer_SRCS
   helpwindow.cpp
   main.cpp
   richpage.cpp
   )

kde4_automoc(${helpviewer_SRCS})

kde4_add_test_executable(helpviewer ${helpviewer_SRCS})

target_link_libraries(helpviewer  ${KDE4_KDECORE_LIBS} kdeprint ${QT_QT3SUPPORT_LIBRARY})


########### next target ###############

set(drawdemo_SRCS drawdemo.cpp )

kde4_automoc(${drawdemo_SRCS})

kde4_add_test_executable(drawdemo ${drawdemo_SRCS})

target_link_libraries(drawdemo  ${KDE4_KDECORE_LIBS} kdeprint )

########### install files ###############





