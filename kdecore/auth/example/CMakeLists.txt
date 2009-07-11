cmake_minimum_required(VERSION 2.6)

project( kauth_example )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package( Qt4 REQUIRED )
find_package( KAuth REQUIRED )
set(QT_USE_QTDBUS true)
include(${QT_USE_FILE})

include_directories(${KAUTH_INCLUDE_DIR} ${CMAKE_BINARY_DIR})

# Auth example application

set( kauth_example_sources
     main.cpp
     mainwindow.cpp )

set( kauth_example_headers
     mainwindow.h )

qt4_wrap_cpp(kauth_example_mocs ${kauth_example_headers})
qt4_wrap_ui(kauth_example_uis mainwindow.ui)

add_executable(kauth_example ${kauth_example_sources} ${kauth_example_mocs} ${kauth_example_uis})
target_link_libraries(kauth_example ${KAUTH_LIBRARY} ${QT_LIBRARIES} )

set_target_properties(kauth_example PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

install(TARGETS kauth_example DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Auth example helper

qt4_wrap_cpp(kauth_helper_mocs helper/helper.h)
add_executable(kauth_helper helper/helper.cpp ${kauth_helper_mocs})
target_link_libraries(kauth_helper ${KAUTH_LIBRARY} ${QT_QTCORE_LIBRARIES})

set_target_properties(kauth_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

kde4_auth_register_helper(kauth_helper org.kde.auth.example root)
kde4_auth_register_actions(kauth_helper org.kde.auth.example actions.ini)
