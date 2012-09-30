#project("plasmapkg")

set(plasmapkg_SRCS
    main.cpp
    plasmapkg.cpp
)

kde4_add_executable(plasmapkg2 ${plasmapkg_SRCS})

target_link_libraries(plasmapkg2 ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS})

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

install(TARGETS plasmapkg2 ${INSTALL_TARGETS_DEFAULT_ARGS})

