include(ECMMarkNonGuiExecutable)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory( pics )

########### next target ###############

add_executable(makekdewidgets makekdewidgets.cpp)
ecm_mark_nongui_executable(makekdewidgets)

target_link_libraries(makekdewidgets KF5::KConfigCore # KConfig + KConfigGroup
                                     KF5::KCoreAddons # KMacroExpander
                                     )

# "export" this target too so we can use the LOCATION property of the imported target in
# FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
install(TARGETS makekdewidgets  EXPORT KDEWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

find_package(Qt5Designer)

if(Qt5Designer_FOUND)

  include_directories(${Qt5Designer_INCLUDE_DIRS})
  # get the name of the generated wrapper script (which sets up LD_LIBRARY_PATH)
  get_target_property(MAKEKDEWIDGETS_EXECUTABLE makekdewidgets LOCATION)

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
    COMMAND "${MAKEKDEWIDGETS_EXECUTABLE}" -o ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets
    MAIN_DEPENDENCY kde.widgets DEPENDS makekdewidgets)

  set(kdewidgets_PART_SRCS
     classpreviews.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.moc
  )

  qt5_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.moc)


  qt5_add_resources(kdewidgets_PART_SRCS kdewidgets.qrc)

  add_library(kdewidgets MODULE ${kdewidgets_PART_SRCS})

  target_link_libraries(kdewidgets KF5::KIOFileWidgets KF5::KPlotting KF5::KDE4Support KF5::KWidgetsAddons KF5::KI18n KF5::KIconThemes KF5::KTextWidgets KF5::ItemViews)
  if(NOT WIN32)
    set_target_properties(kdewidgets PROPERTIES
                                     INSTALL_RPATH_USE_LINK_PATH TRUE
                                     SKIP_BUILD_RPATH TRUE
                                     BUILD_WITH_INSTALL_RPATH TRUE
                                     INSTALL_RPATH ${LIB_INSTALL_DIR}
                         )
  endif(NOT WIN32)

  install(TARGETS kdewidgets  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/designer )

  if(Qt5WebKitWidgets_FOUND)

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp
    COMMAND "${MAKEKDEWIDGETS_EXECUTABLE}" -o ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kdewebkit.widgets
    MAIN_DEPENDENCY kdewebkit.widgets DEPENDS makekdewidgets)

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
  endif(NOT WIN32)

  install(TARGETS kdewebkitwidgets  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/designer )

  endif() # webkit

endif(Qt5Designer_FOUND)

########### install files ###############





