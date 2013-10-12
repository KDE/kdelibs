project("desktoptojson")

add_executable(desktoptojson
    main.cpp
    kconfigtojson.cpp
)

target_link_libraries(desktoptojson
    Qt5::Core
    KF5::KConfigCore
)

# "export" this target too so we can use the LOCATION property of the imported target in
# FindKDE4Internal.cmake to get the full path to the installed executable instead of using
# FIND_PROGRAM(), Alex
install(TARGETS desktoptojson EXPORT KServiceTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
