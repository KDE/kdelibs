set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR} ../ ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

set (kidletest_SRCS
main.cpp
KIdleTest.cpp
)

kde4_add_executable(kidletest ${kidletest_SRCS})
target_link_libraries(kidletest ${KDE4_KDEUI_LIBS}  ${QT_QTCORE_LIBRARY} kutils)
