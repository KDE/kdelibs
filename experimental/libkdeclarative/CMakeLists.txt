project(kdeclarative)

find_package(Qt5Quick REQUIRED)
find_package(Qt5Qml REQUIRED)

include_directories(${KDE4_INCLUDE_DIR}/KDE)

set(kdeclarative_LIB_SRCS
   # configpropertymap.cpp
    qmlobject.cpp
    kdeclarative.cpp
    private/kiconprovider.cpp)

kde4_add_library(kdeclarative SHARED ${kdeclarative_LIB_SRCS})
generate_export_header(kdeclarative)

set_target_properties(kdeclarative PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

target_link_libraries(kdeclarative ${KDE4_KDECORE_LIBS} kdeui ${Qt5Qml_LIBRARIES} ${Qt5Quick_LIBRARIES} ${KCoreAddons_LIBRARIES} ki18n)


## test
set(kdeclarative_TEST_SRCS
    test/test.cpp)

kde4_add_executable(kdeclarativetest ${kdeclarative_TEST_SRCS})

find_package(Qt5Script REQUIRED NO_MODULE)

target_link_libraries(kdeclarativetest kdeclarative ${KDE4_KDECORE_LIBS} Qt5::Widgets ${Qt5Quick_LIBRARIES} ${Qt5Qml_LIBRARIES} Qt5::Script ki18n)

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
