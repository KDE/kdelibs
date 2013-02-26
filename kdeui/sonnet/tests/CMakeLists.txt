set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

########### backgroundtest ###############

set(backgroundtest_SRCS backgroundtest.cpp)
add_executable(backgroundtest ${backgroundtest_SRCS})
ecm_mark_as_test(backgroundtest)
target_link_libraries(backgroundtest ${KDE4_KDEUI_LIBS}  )

########### test_highlighter ###############

set(test_highlighter_SRCS test_highlighter.cpp)
add_executable(test_highlighter ${test_highlighter_SRCS})
ecm_mark_as_test(test_highlighter)
target_link_libraries(test_highlighter ${KDE4_KDEUI_LIBS} ${KCoreAddons_LIBRARIES} )

########### test_configdialog ###############

set(test_configdialog_SRCS test_configdialog.cpp)
add_executable(test_configdialog ${test_configdialog_SRCS})
ecm_mark_as_test(test_configdialog)
target_link_libraries(test_configdialog ${KDE4_KDEUI_LIBS} ${KCoreAddons_LIBRARIES} )

########### test_dictionarycombobox ###############

add_executable(test_dictionarycombobox test_dictionarycombobox.cpp)
ecm_mark_as_test(test_dictionarycombobox)
target_link_libraries(test_dictionarycombobox ${KDE4_KDEUI_LIBS} ${KCoreAddons_LIBRARIES} )
