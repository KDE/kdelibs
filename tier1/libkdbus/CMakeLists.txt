project(libkdbus)

include_directories(
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(libkdbus_SRCS
    kdbusconnectionpool.cpp
    kdbusinterprocesslock.cpp
    kdbusservice.cpp
)

kf5_add_library(kdbus ${LIBRARY_TYPE} ${libkdbus_SRCS})

target_link_libraries(kdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

set_target_properties(kdbus PROPERTIES VERSION   ${KDE_NON_GENERIC_LIB_VERSION}
                                       SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)

install(TARGETS kdbus EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdbusconnectionpool.h
    kdbusinterprocesslock.h
    kdbusservice.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdbus_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

add_subdirectory(tests)

