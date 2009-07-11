cmake_minimum_required(VERSION 2.6)

project( example )

include_directories(${CMAKE_BINARY_DIR}/example)

# Auth example application

set( kauth_example_sources
     main.cpp
     mainwindow.cpp )

set( kauth_example_headers
     mainwindow.h )

qt4_wrap_cpp(kauth_example_mocs ${kauth_example_headers})
qt4_wrap_ui(kauth_example_uis mainwindow.ui)

add_executable(kauth_example ${kauth_example_sources} ${kauth_example_mocs} ${kauth_example_uis})
target_link_libraries(kauth_example kauth ${QT_QTGUI_LIBRARIES})

set_target_properties(kauth_example PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

install(TARGETS kauth_example DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Auth example helper

qt4_wrap_cpp(kauth_helper_mocs helper/helper.h)
add_executable(kauth_helper helper/helper.cpp ${kauth_helper_mocs})
target_link_libraries(kauth_helper kauth )

set_target_properties(kauth_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

kde4_auth_register_helper(org.kde.auth.example kauth_helper root)
kde4_auth_register_actions(org.kde.auth.example actions.ini)
