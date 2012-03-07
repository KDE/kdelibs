set(libkdeqt5staging_SRCS
    qurlpathinfo.cpp
    qsavefile.cpp
)

add_library(kdeqt5staging ${LIBRARY_TYPE} ${libkdeqt5staging_SRCS})

generate_export_header(kdeqt5staging)

target_link_libraries(kdeqt5staging ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kdeqt5staging PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                               SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kdeqt5staging EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

# Install into a kdeqt5staging subdir to prevent clashes with real Qt header files
install(FILES
    qurlpathinfo.h
    qsavefile.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeqt5staging_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdeqt5staging COMPONENT Devel
)

