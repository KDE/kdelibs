find_package(X11)

if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()

configure_file(config-kinterprocesswindowing.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kinterprocesswindowing.h)

set(libkinterprocesswindowing_SRCS
   kusertimestamp.cpp
)


add_library(kinterprocesswindowing ${libkinterprocesswindowing_SRCS})
generate_export_header(kinterprocesswindowing)

target_link_libraries(kinterprocesswindowing
   LINK_PUBLIC ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${kwindowsystem_LIBRARY} ${X11_LIBRARIES} ${kdeqt5staging_LIBRARY}
)

set_target_properties(kinterprocesswindowing PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                                        SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kinterprocesswindowing EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kusertimestamp.h
    ${CMAKE_CURRENT_BINARY_DIR}/kinterprocesswindowing_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
