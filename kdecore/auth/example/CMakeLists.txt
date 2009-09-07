include (MacroKAuth)

# Auth example application

set( kauth_example_sources
     main.cpp
     mainwindow.cpp )

qt4_wrap_ui(kauth_example_uis mainwindow.ui)

kde4_add_executable(kauth_example ${kauth_example_sources} ${kauth_example_uis})
target_link_libraries(kauth_example kdecore ${QT_QTGUI_LIBRARY} kdeui )

# Auth example helper
set(kauth_helper_srcs helper/helper.cpp)
kde4_add_executable(kauth_helper ${kauth_helper_srcs})
target_link_libraries(kauth_helper kdecore)
install(TARGETS kauth_helper DESTINATION ${LIBEXEC_INSTALL_DIR})

kde4_install_auth_helper_files(kauth_helper org.kde.auth.example root)
kde4_install_auth_actions(org.kde.auth.example actions.actions)

# miscellaneous
set_target_properties(kauth_example PROPERTIES COMPILE_FLAGS "-Wall -ggdb")
set_target_properties(kauth_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")