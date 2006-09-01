
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/ ${CMAKE_SOURCE_DIR}/WeaverGui/ ${KDE4_INCLUDES})

set(Jobs_SRCS
    Jobs.cpp
    WeaverObserverTest.cpp
)

set( Jobs_SRCS_UI
    JobsBase.ui
)

kde4_add_ui_files(Jobs_SRCS ${Jobs_SRCS_UI} )

kde4_automoc(${Jobs_SRCS})

kde4_add_executable(Jobs ${Jobs_SRCS})

target_link_libraries(Jobs ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ThreadWeaver ThreadWeaverGui)

install(TARGETS Jobs DESTINATION ${BIN_INSTALL_DIR} )
