include_directories(
   ${CMAKE_SOURCE_DIR}/staging/kio/src/core # kemailsettings.h
   ${CMAKE_BINARY_DIR}/staging/kio/src/include # kio/kio_export.h
)

set(ksendbugmail_SRCS
  main.cpp
  smtp.cpp
  ${CMAKE_SOURCE_DIR}/staging/kio/src/core/kemailsettings.cpp # not worth linking kiocore for just one file
)


kde4_add_executable(ksendbugmail NOGUI ${ksendbugmail_SRCS})

target_link_libraries(ksendbugmail  ${KDE4_KDEUI_LIBS} Qt5::Network ${KCoreAddons_LIBRARIES})

set_target_properties(ksendbugmail PROPERTIES COMPILE_FLAGS -DKIO_EXPORT=)

install(TARGETS ksendbugmail DESTINATION ${LIBEXEC_INSTALL_DIR} )

