configure_file(config-kinterprocesswindowing.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kinterprocesswindowing.h)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(libkinterprocesswindowing_SRCS
   kusertimestamp.cpp
   kmessagebox_kiw.cpp
)


add_library(KInterProcessWindowing ${libkinterprocesswindowing_SRCS})
generate_export_header(KInterProcessWindowing)

target_link_libraries(KInterProcessWindowing
LINK_PUBLIC
   KWidgetsAddons
LINK_PRIVATE
   ${KWindowSystem_LIBRARIES}
   KI18n
)

if (X11_FOUND)
  find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
  target_link_libraries(KInterProcessWindowing LINK_PRIVATE Qt5::X11Extras ${X11_LIBRARIES})
endif()

set_target_properties(KInterProcessWindowing PROPERTIES VERSION   ${KINTERPROCESSWINDOWING_VERSION_STRING}
                                                        SOVERSION ${KINTERPROCESSWINDOWING_SOVERSION}
)

install(TARGETS KInterProcessWindowing EXPORT KInterProcessWindowingTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kusertimestamp.h
    kmessagebox_kiw.h
    ${CMAKE_CURRENT_BINARY_DIR}/kinterprocesswindowing_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
