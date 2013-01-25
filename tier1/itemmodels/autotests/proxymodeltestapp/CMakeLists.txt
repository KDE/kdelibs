
find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Script)

set(proxymodeltestapp_SRCS
    main.cpp
    mainwindow.cpp
    breadcrumbswidget.cpp
    breadcrumbnavigationwidget.cpp
    breadcrumbdirectionwidget.cpp
    checkablewidget.cpp
    selectionpmwidget.cpp
#     statesaverwidget.cpp
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

add_executable(proxymodeltestapp ${proxymodeltestapp_SRCS})

target_link_libraries(proxymodeltestapp
   ItemModels
   proxymodeltestsuite
   ${QT_QTSCRIPT_LIBRARY}
   Qt5::Widgets
)


