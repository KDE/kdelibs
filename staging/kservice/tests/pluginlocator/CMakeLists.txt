project("plugintest")

add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest
    Qt5::Core
    Qt5::DBus
    kdeqt5staging
    KCoreAddons
    KService
)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

#install(TARGETS plugintest ${INSTALL_TARGETS_DEFAULT_ARGS})

