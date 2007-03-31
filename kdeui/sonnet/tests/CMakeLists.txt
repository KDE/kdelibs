if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kdeui/sonnet/ui )

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(backgroundtest_SRCS backgroundtest.cpp)

kde4_automoc(${backgroundtest_SRCS})

kde4_add_executable(backgroundtest RUN_UNINSTALLED ${backgroundtest_SRCS})

target_link_libraries(backgroundtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(test_dialog_SRCS test_dialog.cpp)

kde4_automoc(${test_dialog_SRCS})

kde4_add_executable(test_dialog NOGUI RUN_UNINSTALLED ${test_dialog_SRCS})

target_link_libraries(test_dialog ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_highlighter_SRCS test_highlighter.cpp)

kde4_automoc(${test_highlighter_SRCS})

kde4_add_executable(test_highlighter NOGUI RUN_UNINSTALLED ${test_highlighter_SRCS})

target_link_libraries(test_highlighter ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_configdialog_SRCS test_configdialog.cpp)

kde4_automoc(${test_configdialog_SRCS})

kde4_add_executable(test_configdialog NOGUI RUN_UNINSTALLED ${test_configdialog_SRCS})

target_link_libraries(test_configdialog ${KDE4_KDEUI_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############





