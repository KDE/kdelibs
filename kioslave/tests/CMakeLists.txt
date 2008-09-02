set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/../http)

kde4_add_unit_test(httpheadertokenizetest httpheadertokenizetest.cpp)
target_link_libraries(httpheadertokenizetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
                      ${QT_QTGUI_LIBRARY})
