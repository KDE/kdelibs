project(proxymodeltestapp)

include_directories( ${KDE4_INCLUDES}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/..
  ${PROJECT_SOURCE_DIR}/..
  ${PROJECT_BINARY_DIR}/../tests
  ${PROJECT_SOURCE_DIR}/../tests
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


