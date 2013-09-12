project("plugintest")

add_executable(plugintest
    main.cpp
    plugintest.cpp
)
add_test(kservice-pluginloadertest plugintest)

target_link_libraries(plugintest
    Qt5::Core
#    Qt5::DBus
    KCoreAddons
    KService
)
ecm_mark_as_test(plugintest)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

#install(TARGETS plugintest ${INSTALL_TARGETS_DEFAULT_ARGS})

