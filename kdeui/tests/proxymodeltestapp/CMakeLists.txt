include_directories( ${KDE4_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/../tests
  ${CMAKE_CURRENT_SOURCE_DIR}/../tests
)

set(proxymodeltestapp_SRCS
    main.cpp
    mainwindow.cpp
    ../dynamictreemodel.cpp
    selectionpmwidget.cpp
    descendantpmwidget.cpp
)

kde4_add_executable(proxymodeltestapp ${proxymodeltestapp_SRCS})

target_link_libraries(proxymodeltestapp
   ${KDE4_KDEUI_LIBS}
)


