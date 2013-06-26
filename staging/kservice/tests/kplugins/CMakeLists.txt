project("plugintest")

add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest
    kservice
    KConfigCore
    KCoreAddons
    ki18n
    kdeqt5staging)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

#install(TARGETS plugintest ${INSTALL_TARGETS_DEFAULT_ARGS})

