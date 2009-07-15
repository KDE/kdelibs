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
target_link_libraries(kauth_example ${KAUTH_LIBRARY} ${QT_LIBRARIES}  )

install(TARGETS kauth_example DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Auth example helper

qt4_wrap_cpp(kauth_helper_mocs helper/helper.h)
set(kauth_helper_srcs helper/helper.cpp ${kauth_helper_mocs})

kde4_auth_add_helper(kauth_helper org.kde.auth.example root ${kauth_helper_srcs})
kde4_auth_register_actions(org.kde.auth.example actions.ini)

# miscellaneous
set_target_properties(kauth_example PROPERTIES COMPILE_FLAGS "-Wall -ggdb")
set_target_properties(kauth_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")