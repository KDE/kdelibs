set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
# Tests don't need to go into toplevel/bin, they are fine in the current dir.
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(kwalletwizardtest_SRCS
   kwalletwizardtest.cpp
   ../kwalletwizard.cpp
)
kde4_add_ui_files(kwalletwizardtest_SRCS
   ../kbetterthankdialogbase.ui
   ../kwalletwizardpageexplanation.ui
   ../kwalletwizardpageintro.ui
   ../kwalletwizardpageoptions.ui
   ../kwalletwizardpagepassword.ui
)

kde4_add_executable(kwalletwizardtest TEST ${kwalletwizardtest_SRCS})
target_link_libraries(kwalletwizardtest ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})
