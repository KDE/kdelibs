cmake_minimum_required(VERSION 2.8.5)

project(kdbusaddons)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt4 REQUIRED QtCore QtDBus)

include(${QT_USE_FILE})

set(libkdbusaddons_SRCS
    kdbusconnectionpool.cpp
    kdbusinterprocesslock.cpp
    kdbusservice.cpp
)

add_library(kdbusaddons ${LIBRARY_TYPE} ${libkdbusaddons_SRCS})
generate_export_header(kdbusaddons)

target_link_libraries(kdbusaddons ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

set_target_properties(kdbusaddons PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                             SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kdbusaddons ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kdbusconnectionpool.h
    kdbusinterprocesslock.h
    kdbusservice.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

add_subdirectory(autotests)

