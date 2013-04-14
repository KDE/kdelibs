find_package(X11)

if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()

configure_file(config-kinterprocesswindowing.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kinterprocesswindowing.h)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(libkinterprocesswindowing_SRCS
   kusertimestamp.cpp
   kmessagebox_kiw.cpp
)


add_library(kinterprocesswindowing ${libkinterprocesswindowing_SRCS})
generate_export_header(kinterprocesswindowing)

target_link_libraries(kinterprocesswindowing
   LINK_PUBLIC Qt5::Widgets ${KWindowSystem_LIBRARIES} ${X11_LIBRARIES}
   kwidgets ki18n ${KConfig_CORE_LIBRARY} kdeui
)

if (X11_FOUND)
  find_package(Qt5X11Extras REQUIRED NO_MODULE)
  target_link_libraries(kinterprocesswindowing Qt5::X11Extras)
endif()

set_target_properties(kinterprocesswindowing PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                                        SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kinterprocesswindowing EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kusertimestamp.h
    kmessagebox_kiw.h
    ${CMAKE_CURRENT_BINARY_DIR}/kinterprocesswindowing_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
