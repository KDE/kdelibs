set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(backgroundtest_SRCS backgroundtest.cpp)


kde4_add_executable(backgroundtest TEST  ${backgroundtest_SRCS})

target_link_libraries(backgroundtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(test_dialog_SRCS test_dialog.cpp)


kde4_add_executable(test_dialog TEST  ${test_dialog_SRCS})

target_link_libraries(test_dialog ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_highlighter_SRCS test_highlighter.cpp)


kde4_add_executable(test_highlighter TEST  ${test_highlighter_SRCS})

target_link_libraries(test_highlighter ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_configdialog_SRCS test_configdialog.cpp)


kde4_add_executable(test_configdialog TEST  ${test_configdialog_SRCS})

target_link_libraries(test_configdialog ${KDE4_KDEUI_LIBS} )

########### next target ###############

kde4_add_executable(test_dictionarycombobox TEST test_dictionarycombobox.cpp)
target_link_libraries(test_dictionarycombobox ${KDE4_KDEUI_LIBS})
