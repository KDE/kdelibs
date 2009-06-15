set(ksendbugmail_SRCS
  main.cpp
  smtp.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kemailsettings.cpp # not worth linking kio for just one file
)


kde4_add_executable(ksendbugmail NOGUI ${ksendbugmail_SRCS})

target_link_libraries(ksendbugmail  ${KDE4_KDEUI_LIBS} ${QT_QTNETWORK_LIBRARY})

set_target_properties(ksendbugmail PROPERTIES COMPILE_FLAGS -DKIO_EXPORT=)

install(TARGETS ksendbugmail DESTINATION ${LIBEXEC_INSTALL_DIR} )

