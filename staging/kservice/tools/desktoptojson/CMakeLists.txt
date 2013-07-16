project("desktoptojson")

add_executable(desktoptojson
    main.cpp
    kconfigtojson.cpp
)

target_link_libraries(desktoptojson
    Qt5::Core
    KConfigCore
    KI18N
    kdeqt5staging
)

install(TARGETS desktoptojson ${INSTALL_TARGETS_DEFAULT_ARGS})

