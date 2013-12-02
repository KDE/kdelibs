set(KDEWebKit_LIB_SRCS
    kwebwallet.cpp
    kgraphicswebview.cpp
    kwebpage.cpp
    kwebview.cpp
    kwebpluginfactory.cpp
)

add_library(KDEWebKit ${KDEWebKit_LIB_SRCS})
generate_export_header(KDEWebKit)
add_library(KF5::KDEWebKit ALIAS KDEWebKit)

target_link_libraries(KDEWebKit PUBLIC Qt5::WebKitWidgets
                                PRIVATE KF5::CoreAddons KF5::KWallet KF5::KIOWidgets KF5::KParts)

set_target_properties(KDEWebKit PROPERTIES VERSION   ${KDEWEBKIT_VERSION_STRING}
                                           SOVERSION ${KDEWEBKIT_SOVERSION})

install(TARGETS KDEWebKit EXPORT KDEWebKitTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdewebkit_export.h
    kwebwallet.h
    kgraphicswebview.h
    kwebpluginfactory.h
    kwebpage.h
    kwebview.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

