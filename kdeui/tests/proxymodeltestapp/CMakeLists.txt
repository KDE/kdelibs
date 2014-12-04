set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(proxymodeltestapp_SRCS
    main.cpp
    mainwindow.cpp
    breadcrumbswidget.cpp
    breadcrumbnavigationwidget.cpp
    breadcrumbdirectionwidget.cpp
    checkablewidget.cpp
    selectionpmwidget.cpp
    statesaverwidget.cpp
    kidentityproxymodelwidget.cpp
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
   ${QT_QTNETWORK_LIBRARY}
   ${QT_QTSCRIPT_LIBRARY}
)


