set(KDEWebKit_LIB_SRCS
    kwebwallet.cpp
    kgraphicswebview.cpp
    kwebpage.cpp
    kwebview.cpp
    kwebpluginfactory.cpp
)

add_library(KF5WebKit ${KDEWebKit_LIB_SRCS})
generate_export_header(KF5WebKit BASE_NAME KDEWebKit)
add_library(KF5::WebKit ALIAS KF5WebKit)

target_link_libraries(KF5WebKit PUBLIC Qt5::WebKitWidgets
                                PRIVATE KF5::CoreAddons KF5::Wallet KF5::KIOWidgets KF5::Parts)

set_target_properties(KF5WebKit PROPERTIES VERSION   ${KDEWEBKIT_VERSION_STRING}
                                           SOVERSION ${KDEWEBKIT_SOVERSION}
                                           EXPORT_NAME WebKit
)

install(TARGETS KF5WebKit EXPORT KF5WebKitTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdewebkit_export.h
    kwebwallet.h
    kgraphicswebview.h
    kwebpluginfactory.h
    kwebpage.h
    kwebview.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

