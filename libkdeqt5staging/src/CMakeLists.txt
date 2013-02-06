set(libkdeqt5staging_SRCS
    qurlpathinfo.cpp
)

if (QT5_BUILD AND NOT WIN32)
  set(libkdeqt5staging_SRCS ${libkdeqt5staging_SRCS} qt5only/qx11info.cpp)
  install(FILES qt5only/QX11Info DESTINATION ${INCLUDE_INSTALL_DIR}/kdeqt5staging/QtX11Extras COMPONENT Devel)
  install(FILES qt5only/qx11info_x11.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdeqt5staging COMPONENT Devel)
  include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()

add_library(kdeqt5staging ${libkdeqt5staging_SRCS})

generate_export_header(kdeqt5staging)

target_link_libraries(kdeqt5staging Qt5::Widgets)

set_target_properties(kdeqt5staging PROPERTIES VERSION   ${KDEQT5STAGING_VERSION_STRING}
                                               SOVERSION ${KDEQT5STAGING_SOVERSION}
)


# Install into a kdeqt5staging subdir to prevent clashes with real Qt header files
install(FILES
    qurlpathinfo.h
    qtest_staging.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeqt5staging_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdeqt5staging COMPONENT Devel
)

###################

install(TARGETS kdeqt5staging EXPORT kdeqt5stagingLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
