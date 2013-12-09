project("plugintest")

add_executable(plugintest
    main.cpp
    plugintest.cpp
)
add_test(kservice-pluginloadertest plugintest)

target_link_libraries(plugintest
    Qt5::Core
    KF5::Service
)
ecm_mark_as_test(plugintest)
