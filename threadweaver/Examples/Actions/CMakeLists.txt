
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/ ${CMAKE_SOURCE_DIR}/Action/ ${KDE4_INCLUDES})

set(Example_SRCS
    Example.cpp
    main.cpp
)

set( Example_SRCS_UI
Example.ui
)

qt4_wrap_ui(Example_SRCS ${Example_SRCS_UI} )

QT4_AUTOMOC(${Example_SRCS})

add_executable(Example ${Example_SRCS})

target_link_libraries(Example ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ThreadWeaver ThreadWeaverAction)

install(TARGETS Example DESTINATION bin )
