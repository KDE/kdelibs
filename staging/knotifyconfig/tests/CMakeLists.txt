project(knotifytest)

########### knotifytest ###############

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(knotifytest_SRCS main.cpp knotifytestwindow.cpp )

qt5_wrap_ui(knotifytest_SRCS knotifytestview.ui)

add_executable(knotifytest ${knotifytest_SRCS})
ecm_mark_as_test(knotifytest)

target_link_libraries(knotifytest Qt5::Core Qt5::Test KNotifyConfig KF5::KNotifications KF5::KCoreAddons KF5::KI18n KF5::XmlGui KF5::KCompletion)
