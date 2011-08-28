include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/ ${CMAKE_SOURCE_DIR}/WeaverGui/ ${KDE4_INCLUDES})
include_directories( ../../Weaver/ )
include_directories( ../../WeaverGui/ )

set(Jobs_SRCS
    Jobs.cpp
    WeaverObserverTest.cpp
)

set( Jobs_SRCS_UI
    JobsBase.ui
)

qt4_wrap_ui(Jobs_SRCS ${Jobs_SRCS_UI} )


add_executable(Jobs ${Jobs_SRCS})

target_link_libraries(Jobs ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} threadweaver ThreadWeaverGui)

install(TARGETS Jobs DESTINATION ${BIN_INSTALL_DIR} )
