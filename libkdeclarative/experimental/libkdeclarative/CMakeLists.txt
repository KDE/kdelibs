project(kdeclarative)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(kdeclarative_LIB_SRCS
    kdeclarative.cpp
    private/engineaccess.cpp)

kde4_add_library(kdeclarative ${LIBRARY_TYPE} ${kdeclarative_LIB_SRCS})

set_target_properties(kdeclarative PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

target_link_libraries(kdeclarative ${QT_QTDECLARATIVE_LIBRARY})


## test
set(kdeclarative_TEST_SRCS
    test/test.cpp)

kde4_add_executable(kdeclarativetest ${kdeclarative_TEST_SRCS})

target_link_libraries(kdeclarativetest kdeclarative ${KDE4_KDECORE_LIBS} ${QT_QTDECLARATIVE_LIBRARY})

## install

set(kdeclarative_LIB_HEADERS
    kdeclarative.h)

install(FILES ${kdeclarative_LIB_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/ COMPONENT Devel)

install(TARGETS kdeclarative EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


install(FILES "test/test.qml" DESTINATION ${CMAKE_BINARY_DIR})