set(libkdeqt5staging_SRCS
    qurlpathinfo.cpp
)

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
