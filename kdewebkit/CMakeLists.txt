project(kdewebkit)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/kdewebkit)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

add_subdirectory(includes)
add_subdirectory(cmake)

set(kdewebkit_LIB_SRCS
    kgraphicswebview.cpp
    kwebpage.cpp
    kwebview.cpp
    kwebpluginfactory.cpp
    networkaccessmanager_p.cpp
)

kde4_add_library(kdewebkit SHARED ${kdewebkit_LIB_SRCS})

target_link_libraries(kdewebkit
    ${KDE4_KPARTS_LIBS}
    ${QT_QTWEBKIT_LIBRARY}
)

set_target_properties(kdewebkit PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

#install(TARGETS kdewebkit EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}) ### for kdelibs
install(TARGETS kdewebkit ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdewebkit_export.h
    kgraphicswebview.h
    kwebpluginfactory.h
    kwebpage.h
    kwebview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdewebkit COMPONENT Devel)
