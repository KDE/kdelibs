
set(platformtheme_SRCS
   kdeplatformtheme.cpp
   kfontsettingsdata.cpp
   khintssettings.cpp
   kdeplatformfiledialoghelper.cpp
   main.cpp
)

include_directories(
   ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_library(KDEPlatformTheme MODULE ${platformtheme_SRCS})

# We link to KIO to pull in the qfiledialog hooks
target_link_libraries(KDEPlatformTheme PRIVATE Qt5::DBus KF5::ConfigWidgets KF5::ConfigCore KF5::IconThemes KF5::KIOFileWidgets KF5::I18n)

install(TARGETS KDEPlatformTheme DESTINATION ${QT_PLUGIN_INSTALL_DIR}/platformthemes)
