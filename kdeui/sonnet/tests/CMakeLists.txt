set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### backgroundtest ###############

set(backgroundtest_SRCS backgroundtest.cpp)
kde4_add_executable(backgroundtest TEST  ${backgroundtest_SRCS})
target_link_libraries(backgroundtest ${KDE4_KDEUI_LIBS} kcoreaddons)

########### test_highlighter ###############

set(test_highlighter_SRCS test_highlighter.cpp)
kde4_add_executable(test_highlighter TEST  ${test_highlighter_SRCS})
target_link_libraries(test_highlighter ${KDE4_KDEUI_LIBS} kcoreaddons)

########### test_configdialog ###############

set(test_configdialog_SRCS test_configdialog.cpp)
kde4_add_executable(test_configdialog TEST  ${test_configdialog_SRCS})
target_link_libraries(test_configdialog ${KDE4_KDEUI_LIBS} kcoreaddons)

########### test_dictionarycombobox ###############

kde4_add_executable(test_dictionarycombobox TEST test_dictionarycombobox.cpp)
target_link_libraries(test_dictionarycombobox ${KDE4_KDEUI_LIBS} kcoreaddons)
