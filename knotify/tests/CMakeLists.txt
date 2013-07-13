set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
# This can be used for finding data files in the source dir, without installing them
project(knotifytest)

########### knotifytest ###############

set(knotifytest_SRCS main.cpp knotifytestwindow.cpp )

kde4_add_ui_files(knotifytest_SRCS knotifytestview.ui)


kde4_add_executable(knotifytest TEST ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${KDE4_KIO_LIBS} Qt5::Test knotifyconfig ${KCoreAddons_LIBRARIES} ki18n KWidgets)

#the binary file doesn't need to be installed
#install(TARGETS knotifytest DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############


