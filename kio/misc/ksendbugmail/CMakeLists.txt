set(ksendbugmail_SRCS
  main.cpp
  smtp.cpp
)

kde4_automoc(${ksendbugmail_SRCS})

kde4_add_executable(ksendbugmail NOGUI ${ksendbugmail_SRCS})

target_link_libraries(ksendbugmail  ${KDE4_KDEUI_LIBS} kio )

install(TARGETS ksendbugmail DESTINATION ${LIBEXEC_INSTALL_DIR} )

