remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(ksendbugmail_SRCS
  main.cpp
  smtp.cpp
)

add_executable(ksendbugmail ${ksendbugmail_SRCS})
ecm_mark_nongui_executable(ksendbugmail)

target_link_libraries(ksendbugmail
   Qt5::Widgets
   Qt5::Network
   KF5::KConfigCore # KEmailSettings
   KF5::KI18n
)

install(TARGETS ksendbugmail DESTINATION ${LIBEXEC_INSTALL_DIR} )

