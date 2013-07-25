
project(kdewidgets)

add_subdirectory( pics )
add_subdirectory( tests )

########### next target ###############

set(makekdewidgets_SRCS makekdewidgets.cpp )


kde4_add_executable(makekdewidgets NOGUI ${makekdewidgets_SRCS})

target_link_libraries(makekdewidgets kdeqt5staging KConfigCore ${KCoreAddons_LIBRARIES} KI18n)

# "export" this target too so we can use the LOCATION property of the imported target in
# FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
install(TARGETS makekdewidgets  EXPORT kdelibsToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

find_package(Qt5Designer)

if(Qt5Designer_FOUND)

  include_directories(${Qt5Designer_INCLUDE_DIRS})
  # get the name of the generated wrapper script (which sets up LD_LIBRARY_PATH)
  get_target_property(MAKEKDEWIDGETS_EXECUTABLE makekdewidgets WRAPPER_SCRIPT)

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

  kde4_add_plugin(kdewidgets ${kdewidgets_PART_SRCS})

  target_link_libraries(kdewidgets  ${KDE4_KIO_LIBS}  ${KPlotting_LIBRARIES} KWidgets KDE4Attic KDE4Support ${KWidgetsAddons_LIBRARIES} KI18n)
  if(NOT WIN32)
    set_target_properties(kdewidgets PROPERTIES
                                     INSTALL_RPATH_USE_LINK_PATH TRUE
                                     SKIP_BUILD_RPATH TRUE
                                     BUILD_WITH_INSTALL_RPATH TRUE
                                     INSTALL_RPATH ${LIB_INSTALL_DIR}
                         )
  endif(NOT WIN32)

  install(TARGETS kdewidgets  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/designer )

  # deprecated widgets
  if(NOT KDE_NO_DEPRECATED)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp
      COMMAND "${MAKEKDEWIDGETS_EXECUTABLE}" -o ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kdedeprecated.widgets
      MAIN_DEPENDENCY kdedeprecated.widgets DEPENDS makekdewidgets)

    set(kdedeprecated_PART_SRCS
       classpreviews.cpp
       ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp
       ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.moc
    )

    qt5_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.moc)


    qt5_add_resources(kdedeprecated_PART_SRCS kdedeprecated.qrc)

    kde4_add_plugin(kdedeprecated ${kdedeprecated_PART_SRCS})

    target_link_libraries(kdedeprecated KDE4Support ${KDE4_KIO_LIBS} KWidgets KDE4Attic)
    if(NOT WIN32)
      set_target_properties(kdedeprecated PROPERTIES
                                          INSTALL_RPATH_USE_LINK_PATH TRUE
                                          SKIP_BUILD_RPATH TRUE
                                          BUILD_WITH_INSTALL_RPATH TRUE
                                          INSTALL_RPATH ${LIB_INSTALL_DIR}
                           )
    endif(NOT WIN32)

    install(TARGETS kdedeprecated  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/designer )
  endif(NOT KDE_NO_DEPRECATED)

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

  kde4_add_plugin(kdewebkitwidgets ${kdewebkitwidgets_PART_SRCS})

  target_link_libraries(kdewebkitwidgets ${KDE4_KDEUI_LIBS} ${KDE4_KDEWEBKIT_LIBS} Qt5::WebKitWidgets)
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





