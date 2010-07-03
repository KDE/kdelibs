set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

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
    breadcrumbswidget.cpp
    breadcrumbnavigationwidget.cpp
    breadcrumbdirectionwidget.cpp
    selectionpmwidget.cpp
    statesaverwidget.cpp
    recursivefilterpmwidget.cpp
    descendantpmwidget.cpp
    reparentingpmwidget.cpp
    kreparentingproxymodel.cpp
    proxymodeltestwidget.cpp
    proxyitemselectionwidget.cpp
    lessthanwidget.cpp
    modelcommanderwidget.cpp
    scriptablereparentingwidget.cpp
    matchcheckingwidget.cpp
)

kde4_add_executable(proxymodeltestapp TEST ${proxymodeltestapp_SRCS})

target_link_libraries(proxymodeltestapp
   ${KDE4_KDEUI_LIBS}
   proxymodeltestsuite
   ${QT_QTSCRIPT_LIBRARY}
)


