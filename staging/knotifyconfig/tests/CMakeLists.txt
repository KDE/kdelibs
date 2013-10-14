set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
# This can be used for finding data files in the source dir, without installing them
project(knotifytest)

########### knotifytest ###############

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(knotifytest_SRCS main.cpp knotifytestwindow.cpp )

qt5_wrap_ui(knotifytest_SRCS knotifytestview.ui)

add_executable(knotifytest ${knotifytest_SRCS})
ecm_mark_as_test(knotifytest)

target_link_libraries(knotifytest ${KDE4_KIO_LIBS} Qt5::Core Qt5::Test KNotifyConfig KF5::KNotifications KF5::KCoreAddons KF5::KI18n KF5::XmlGui KF5::KCompletion)

#the binary file doesn't need to be installed
#install(TARGETS knotifytest DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############


