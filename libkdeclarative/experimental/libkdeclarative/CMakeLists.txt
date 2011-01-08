find_package(KDE4 REQUIRED)

include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(kdeclarative_LIB_SRCS
    kdeclarative.cpp)

kde4_add_library(kdeclarative ${LIBRARY_TYPE} ${kdeclarative_LIB_SRCS})

target_link_libraries(kdeclarative ${QT_QTDECLARATIVE_LIBRARY})

## install

set(kdeclarative_LIB_HEADERS
    kdeclarative.h)

install(FILES ${kdeclarative_LIB_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/ COMPONENT Devel)

install(TARGETS kdeclarative EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
