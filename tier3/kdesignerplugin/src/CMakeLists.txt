include(ECMMarkNonGuiExecutable)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory( pics )

########### next target ###############

add_executable(kgendesignerplugin kgendesignerplugin.cpp)
ecm_mark_nongui_executable(kgendesignerplugin)

target_link_libraries(kgendesignerplugin KF5::ConfigCore # KConfig + KConfigGroup
                                         KF5::CoreAddons # KMacroExpander
                                         )

# "export" this target too so we can use the LOCATION property of the imported target in
# FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
install(TARGETS kgendesignerplugin  EXPORT KF5DesignerPluginTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

find_package(Qt5Designer)

if(Qt5Designer_FOUND)

  include_directories(${Qt5Designer_INCLUDE_DIRS})

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
    COMMAND kgendesignerplugin -o ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets
    MAIN_DEPENDENCY kde.widgets)

  set(kf5widgets_PART_SRCS
     classpreviews.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.moc
  )

  qt5_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.moc)


  qt5_add_resources(kf5widgets_PART_SRCS kdewidgets.qrc)

  add_library(kf5widgets MODULE ${kf5widgets_PART_SRCS})

  target_link_libraries(kf5widgets KF5::KIOFileWidgets KF5::Plotting KF5::WidgetsAddons KF5::I18n KF5::IconThemes KF5::TextWidgets KF5::ItemViews)
  if(NOT WIN32)
    set_target_properties(kf5widgets PROPERTIES
                                     INSTALL_RPATH_USE_LINK_PATH TRUE
                                     SKIP_BUILD_RPATH TRUE
                                     BUILD_WITH_INSTALL_RPATH TRUE
                                     INSTALL_RPATH ${LIB_INSTALL_DIR}
                         )
  endif()

  install(TARGETS kf5widgets DESTINATION ${QT_PLUGIN_INSTALL_DIR}/designer)

  if(Qt5WebKitWidgets_FOUND)

     add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp
       COMMAND kgendesignerplugin -o ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kdewebkit.widgets
       MAIN_DEPENDENCY kdewebkit.widgets)

     set(kdewebkitwidgets_PART_SRCS
        classpreviews.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.moc
     )

     qt5_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.moc)


     qt5_add_resources(kdewebkitwidgets_PART_SRCS kdewebkitwidgets.qrc)

     add_library(kdewebkitwidgets MODULE ${kdewebkitwidgets_PART_SRCS})

     target_link_libraries(kdewebkitwidgets KDEWebKit Qt5::WebKitWidgets)
     if(NOT WIN32)
       set_target_properties(kdewebkitwidgets PROPERTIES
                                        INSTALL_RPATH_USE_LINK_PATH TRUE
                                        SKIP_BUILD_RPATH TRUE
                                        BUILD_WITH_INSTALL_RPATH TRUE
                                        INSTALL_RPATH ${LIB_INSTALL_DIR}
                            )
     endif()

     install(TARGETS kdewebkitwidgets DESTINATION ${QT_PLUGIN_INSTALL_DIR}/designer )

  endif() # webkit

endif()

########### install files ###############





