project("kconfigtojson")

add_executable(kconfigtojson
    main.cpp
    kconfigtojson.cpp
)

target_link_libraries(kconfigtojson
    KConfigCore
    ki18n
    kdeqt5staging
)

install(TARGETS kconfigtojson ${INSTALL_TARGETS_DEFAULT_ARGS})

