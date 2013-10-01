project(kdeclarative)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Quick Qml)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
		    ${CMAKE_CURRENT_BINARY_DIR}/..
		    )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kdeclarative_LIB_SRCS
    configpropertymap.cpp
    qmlobject.cpp
    kdeclarative.cpp
    private/rootcontext.cpp
    private/kiconprovider.cpp)

add_library(kdeclarative SHARED ${kdeclarative_LIB_SRCS})
generate_export_header(kdeclarative)

set_target_properties(kdeclarative PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

target_link_libraries(kdeclarative ${Qt5Qml_LIBRARIES} ${Qt5Quick_LIBRARIES} ${KCoreAddons_LIBRARIES} KConfigGui KI18n KIconThemes)


## test
#TODO: move into a tests subdirectory
add_executable(kdeclarativetest test/test.cpp)

find_package(Qt5Script 5.2.0 REQUIRED NO_MODULE)

target_link_libraries(kdeclarativetest kdeclarative Qt5::Widgets ${Qt5Quick_LIBRARIES} ${Qt5Qml_LIBRARIES} Qt5::Script KI18n)

## install

set(kdeclarative_LIB_HEADERS
    kdeclarative.h
    qmlobject.h
    configpropertymap.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeclarative_export.h
    )

install(FILES ${kdeclarative_LIB_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kdeclarative/ COMPONENT Devel)

install(TARGETS kdeclarative EXPORT kdeclarativeTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


install(
   EXPORT kdeclarativeTargets
   DESTINATION ${LIB_INSTALL_DIR}/cmake/KDeclarative
   FILE kdeclarativeLibraryTargets.cmake
   )

configure_file(kdeclarativeConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/kdeclarativeConfig.cmake" @ONLY)
configure_file(kdeclarativeConfigVersion.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/kdeclarativeConfigVersion.cmake" @ONLY)

install(
   FILES ${CMAKE_CURRENT_BINARY_DIR}/kdeclarativeConfig.cmake
         ${CMAKE_CURRENT_BINARY_DIR}/kdeclarativeConfigVersion.cmake
         DESTINATION ${LIB_INSTALL_DIR}/cmake/KDeclarative
)
