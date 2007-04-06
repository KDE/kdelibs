
if (KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(kwalletwizardtest_SRCS
   kwalletwizardtest.cpp
   ../kwalletwizard.cpp
)
kde4_automoc(${kwalletwizardtest_SRCS})
kde4_add_test(kwalletwizardtest ${kwalletwizardtest_SRCS})
target_link_libraries(kwalletwizardtest ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})


endif (KDE4_BUILD_TESTS)
