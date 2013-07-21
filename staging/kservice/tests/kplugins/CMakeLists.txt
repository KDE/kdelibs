project("plugintest")

include(ECMMarkAsTest)


set(basicplugin_SRCS
    basicplugin.cpp
    )

add_library(basicplugin SHARED ${basicplugin_SRCS})

target_link_libraries(basicplugin
    KService
    KConfigCore
    KCoreAddons
    KI18n)

#install(TARGETS basicplugin DESTINATION ${PLUGIN_INSTALL_DIR}/kplugins)

add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest
    ${QT_QTTEST_LIBRARY}
    KService
    KConfigCore
    KCoreAddons
    KI18n
    kdeqt5staging
)

add_test(kservice-plugintest plugintest)
ecm_mark_as_test(plugintest)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

#install(TARGETS plugintest ${INSTALL_TARGETS_DEFAULT_ARGS})

