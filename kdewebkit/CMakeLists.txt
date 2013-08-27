project(kdewebkit)

add_feature_info(QtWebkitWidgets Qt5WebKitWidgets_FOUND "Needed for building kdelibs" "http://trac.webkit.org/wiki/QtWebKit")

if(NOT Qt5WebKitWidgets_FOUND)
  message(FATAL_ERROR "The QtWebkit module is required.")
endif()

set(kdewebkit_LIB_SRCS
    kwebwallet.cpp
    kgraphicswebview.cpp
    kwebpage.cpp
    kwebview.cpp
    kwebpluginfactory.cpp
)

add_library(kdewebkit ${LIBRARY_TYPE}
    ${kdewebkit_LIB_SRCS}
)
generate_export_header(kdewebkit)

target_link_libraries(kdewebkit ${KDE4_KPARTS_LIBS} Qt5::WebKitWidgets ${KCoreAddons_LIBRARIES} KWallet KI18n KConfigGui)
target_link_libraries(kdewebkit LINK_INTERFACE_LIBRARIES kparts kio Qt5::WebKitWidgets Qt5::Network Qt5::Widgets)

set_target_properties(kdewebkit PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

install(TARGETS kdewebkit EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdewebkit_export.h
    kwebwallet.h
    kgraphicswebview.h
    kwebpluginfactory.h
    kwebpage.h
    kwebview.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
