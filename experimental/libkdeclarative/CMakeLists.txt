project(kdeclarative)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KDECLARATIVE_NO_KIO TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kdeclarative.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdeclarative.h)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_KIO_INCLUDES}
   ${CMAKE_SOURCE_DIR}/kio
   )

if(BUILDING_EXPERIMENTAL_SEPARATELY)
   include_directories(${KDE4_INCLUDES})
endif()

set(kdeclarative_LIB_SRCS
    kdeclarative.cpp
    private/engineaccess.cpp
    private/kiconprovider.cpp
    bindings/qscriptnonguibookkeeping.cpp
    bindings/i18n.cpp
    bindings/icon.cpp
    bindings/url.cpp)

kde4_add_library(kdeclarative SHARED ${kdeclarative_LIB_SRCS})

set_target_properties(kdeclarative PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

target_link_libraries(kdeclarative LINK_PRIVATE ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTSCRIPT_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})


## test
set(kdeclarative_TEST_SRCS
    test/test.cpp)

kde4_add_executable(kdeclarativetest ${kdeclarative_TEST_SRCS})

target_link_libraries(kdeclarativetest kdeclarative ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTSCRIPT_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})

## install

set(kdeclarative_LIB_HEADERS
    kdeclarative.h
    kdeclarative_export.h)

install(FILES ${kdeclarative_LIB_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/ COMPONENT Devel)

install(TARGETS kdeclarative EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


install(
   EXPORT kdelibsLibraryTargets
   DESTINATION ${LIB_INSTALL_DIR}/cmake/KDeclarative
   FILE KDeclarativeLibraryTargets.cmake
   )

configure_file(KDeclarativeConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KDeclarativeConfig.cmake" @ONLY)
configure_file(KDeclarativeConfigVersion.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KDeclarativeConfigVersion.cmake" @ONLY)

install(
   FILES ${CMAKE_CURRENT_BINARY_DIR}/KDeclarativeConfig.cmake
         ${CMAKE_CURRENT_BINARY_DIR}/KDeclarativeConfigVersion.cmake
         DESTINATION ${LIB_INSTALL_DIR}/cmake/KDeclarative
)
