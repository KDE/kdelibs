project("plugintest")

# set(kqpluginfactory_SRCS
#     kqpluginfactory.cpp
#     )
#
# add_library(kqpluginfactory SHARED ${kqpluginfactory_SRCS})
#
# target_link_libraries(kqpluginfactory
#     kservice
#     KConfigCore
#     KCoreAddons
#     ki18n)

#install(TARGETS kqpluginfactory DESTINATION ${PLUGIN_INSTALL_DIR}/kplugins)

add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest
    kservice
    KConfigCore
    KCoreAddons
    KI18n
    kdeqt5staging)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

#install(TARGETS plugintest ${INSTALL_TARGETS_DEFAULT_ARGS})

