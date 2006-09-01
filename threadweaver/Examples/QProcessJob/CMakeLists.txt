
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/ ${CMAKE_SOURCE_DIR}/Action/ ${KDE4_INCLUDES})

set(QProcessJob_SRCS
    QProcessJob.cpp
    MainWindow.cpp
    main.cpp
)

set( QProcessJob_SRCS_UI
    MainWindowUi.ui
)

qt4_wrap_ui(QProcessJob_SRCS ${QProcessJob_SRCS_UI} )

QT4_AUTOMOC(${QProcessJob_SRCS})

add_executable(QProcessJob ${QProcessJob_SRCS})

target_link_libraries(QProcessJob ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ThreadWeaver)

install(TARGETS QProcessJob DESTINATION bin )
