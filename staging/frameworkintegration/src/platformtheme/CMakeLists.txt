
set(platformtheme_SRCS
   kdeplatformtheme.cpp
   kfontsettingsdata.cpp
   khintssettings.cpp
   main.cpp
)

include_directories(
   ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

add_library(KDEPlatformTheme MODULE ${platformtheme_SRCS})
set_target_properties(KDEPlatformTheme PROPERTIES PREFIX "") # remove lib prefix

# We link to KIO to pull in the qfiledialog hooks
target_link_libraries(KDEPlatformTheme LINK_PRIVATE Qt5::DBus KConfigWidgets KF5::KConfigCore KIconThemes)

install(TARGETS KDEPlatformTheme DESTINATION ${QT_PLUGIN_INSTALL_DIR}/platformthemes)
