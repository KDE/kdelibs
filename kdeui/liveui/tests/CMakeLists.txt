
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(tst_kliveui_SRCS tst_kliveui.cpp)

kde4_automoc(${tst_kliveui_SRCS})

kde4_add_executable(kliveuitest RUN_UNINSTALLED ${tst_kliveui_SRCS})

target_link_libraries(kliveuitest ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(tst_xmlguihandler_SRCS tst_xmlguihandler.cpp)

kde4_automoc(${tst_xmlguihandler_SRCS})

kde4_add_executable(kxmlguihandlertest RUN_UNINSTALLED ${tst_xmlguihandler_SRCS})

target_link_libraries(kxmlguihandlertest ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

endif(KDE4_BUILD_TESTS)

########### install files ###############





