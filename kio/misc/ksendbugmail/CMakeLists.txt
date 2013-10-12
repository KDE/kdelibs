remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_SOURCE_DIR}/staging/kio/src/core # kemailsettings.h
   ${CMAKE_BINARY_DIR}/staging/kio/src/include # kio/kio_export.h
)

set(ksendbugmail_SRCS
  main.cpp
  smtp.cpp
  ${CMAKE_SOURCE_DIR}/staging/kio/src/core/kemailsettings.cpp # not worth linking kiocore for just one file
)

add_executable(ksendbugmail ${ksendbugmail_SRCS})
ecm_mark_nongui_executable(ksendbugmail)

target_link_libraries(ksendbugmail Qt5::Widgets Qt5::Network KConfigCore KF5::KI18n KF5::KCoreAddons)

set_target_properties(ksendbugmail PROPERTIES COMPILE_FLAGS -DKIO_EXPORT=)

install(TARGETS ksendbugmail DESTINATION ${LIBEXEC_INSTALL_DIR} )

