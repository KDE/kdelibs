project(knotifytest)

########### knotifytest ###############

include(ECMMarkAsTest)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(knotifytest_SRCS main.cpp knotifytestwindow.cpp )

qt5_wrap_ui(knotifytest_SRCS knotifytestview.ui)

add_executable(knotifytest ${knotifytest_SRCS})
ecm_mark_as_test(knotifytest)

target_link_libraries(knotifytest
   Qt5::Core
   Qt5::Test
   KF5::NotifyConfig
   KF5::Notifications
   KF5::I18n
   KF5::XmlGui
   KF5::Completion
)
