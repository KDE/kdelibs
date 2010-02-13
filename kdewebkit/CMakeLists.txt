project(kdewebkit)

include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${KDE4_KDEUI_INCLUDES}
 ${KDE4_KIO_INCLUDES}
)

set(kdewebkit_LIB_SRCS
    kwebwallet.cpp
    kgraphicswebview.cpp
    kwebpage.cpp
    kwebview.cpp
    kwebpluginfactory.cpp
)

kde4_add_library(kdewebkit SHARED 
    ${kdewebkit_LIB_SRCS} 
)

target_link_libraries(kdewebkit
    ${KDE4_KPARTS_LIBS}
    ${QT_QTWEBKIT_LIBRARY}
)

set_target_properties(kdewebkit PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

install(TARGETS kdewebkit EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdewebkit_export.h
    kwebwallet.h
    kgraphicswebview.h
    kwebpluginfactory.h
    kwebpage.h
    kwebview.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
