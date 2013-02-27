
include_directories(
   ${Qt5Gui_INCLUDE_DIRS}
   ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
   ${kguiaddons_BINARY_DIR}/src
   ${kguiaddons_BINARY_DIR}/src/colors
   ${kguiaddons_SOURCE_DIR}/src/colors
   ${KConfig_INCLUDE_DIRS}
   ${kwidgets_BINARY_DIR}/src
   ${kwidgets_SOURCE_DIR}/src/icons
)

set(platformtheme_SRCS
   kdeplatformtheme.cpp
   main.cpp
)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

add_library(kdeplatformtheme MODULE ${platformtheme_SRCS})
set_target_properties(kdeplatformtheme PROPERTIES PREFIX "") # remove lib prefix

# We link to KIO to pull in the qfiledialog hooks
target_link_libraries(kdeplatformtheme ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KIO_LIBS} ${KConfig_CORE_LIBRARY} kguiaddons)

install(TARGETS kdeplatformtheme DESTINATION ${QT_PLUGIN_INSTALL_DIR}/platformthemes)
