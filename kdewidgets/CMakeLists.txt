
project(kdewidgets)

add_subdirectory( pics )
add_subdirectory( tests )

include_directories(
  ${KDE4_KIO_INCLUDES}
 )


########### next target ###############

set(makekdewidgets_SRCS makekdewidgets.cpp )


kde4_add_executable(makekdewidgets NOGUI ${makekdewidgets_SRCS})

target_link_libraries(makekdewidgets  ${KDE4_KDECORE_LIBS} )

# "export" this target too so we can use the LOCATION property of the imported target in
# FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
install(TARGETS makekdewidgets  EXPORT kdelibsToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

if(QT_QTDESIGNER_FOUND)
  # get the name of the generated wrapper script (which sets up LD_LIBRARY_PATH)
  get_target_property(MAKEKDEWIDGETS_EXECUTABLE makekdewidgets WRAPPER_SCRIPT)

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
    COMMAND "${MAKEKDEWIDGETS_EXECUTABLE}" -o ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets
    MAIN_DEPENDENCY kde.widgets DEPENDS makekdewidgets)

  set(kdewidgets_PART_SRCS
     classpreviews.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp
  )

  qt4_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdewidgets.moc)


  qt4_add_resources(kdewidgets_PART_SRCS kdewidgets.qrc)

  kde4_add_plugin(kdewidgets ${kdewidgets_PART_SRCS})

  target_link_libraries(kdewidgets  ${KDE4_KIO_LIBS})
  if(NOT WIN32)
    set_target_properties(kdewidgets PROPERTIES
                                     INSTALL_RPATH_USE_LINK_PATH TRUE
                                     SKIP_BUILD_RPATH TRUE
                                     BUILD_WITH_INSTALL_RPATH TRUE
                                     INSTALL_RPATH ${LIB_INSTALL_DIR}
                         )
  endif(NOT WIN32)

  install(TARGETS kdewidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

  # deprecated widgets
  if(NOT KDE_NO_DEPRECATED)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp
      COMMAND "${MAKEKDEWIDGETS_EXECUTABLE}" -o ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kdedeprecated.widgets
      MAIN_DEPENDENCY kdedeprecated.widgets DEPENDS makekdewidgets)

    set(kdedeprecated_PART_SRCS
       classpreviews.cpp
       ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp
    )

    qt4_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdedeprecated.moc)


    qt4_add_resources(kdedeprecated_PART_SRCS kdedeprecated.qrc)

    kde4_add_plugin(kdedeprecated ${kdedeprecated_PART_SRCS})

    target_link_libraries(kdedeprecated  ${KDE4_KIO_LIBS})
    if(NOT WIN32)
      set_target_properties(kdedeprecated PROPERTIES
                                          INSTALL_RPATH_USE_LINK_PATH TRUE
                                          SKIP_BUILD_RPATH TRUE
                                          BUILD_WITH_INSTALL_RPATH TRUE
                                          INSTALL_RPATH ${LIB_INSTALL_DIR}
                           )
    endif(NOT WIN32)

    install(TARGETS kdedeprecated  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )
  endif(NOT KDE_NO_DEPRECATED)


  # kdewebkit widgets
  include_directories(
    ${CMAKE_SOURCE_DIR}/kdewebkit
  )

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp
    COMMAND "${MAKEKDEWIDGETS_EXECUTABLE}" -o ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kdewebkit.widgets
    MAIN_DEPENDENCY kdewebkit.widgets DEPENDS makekdewidgets)

  set(kdewebkitwidgets_PART_SRCS
     classpreviews.cpp
     ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp
  )

  qt4_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.cpp ${CMAKE_CURRENT_BINARY_DIR}/kdewebkitwidgets.moc)


  qt4_add_resources(kdewebkitwidgets_PART_SRCS kdewebkitwidgets.qrc)

  kde4_add_plugin(kdewebkitwidgets ${kdewebkitwidgets_PART_SRCS})

  target_link_libraries(kdewebkitwidgets ${KDE4_KDEUI_LIBS} ${KDE4_KDEWEBKIT_LIBS} ${QT_QTWEBKIT_LIBRARY})
  if(NOT WIN32)
    set_target_properties(kdewebkitwidgets PROPERTIES
                                     INSTALL_RPATH_USE_LINK_PATH TRUE
                                     SKIP_BUILD_RPATH TRUE
                                     BUILD_WITH_INSTALL_RPATH TRUE
                                     INSTALL_RPATH ${LIB_INSTALL_DIR}
                         )
  endif(NOT WIN32)

  install(TARGETS kdewebkitwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )


  if (QT_QT3SUPPORT_FOUND)

     include_directories(
       ${CMAKE_SOURCE_DIR}/kde3support
       ${CMAKE_SOURCE_DIR}/kde3support/kdeui
     )

     add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kde3supportwidgets.cpp
       COMMAND "${MAKEKDEWIDGETS_EXECUTABLE}" -o ${CMAKE_CURRENT_BINARY_DIR}/kde3supportwidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kde3support.widgets
       MAIN_DEPENDENCY kde3support.widgets DEPENDS makekdewidgets)

     set(kde3supportwidgets_PART_SRCS
        classpreviews.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/kde3supportwidgets.cpp
     )

     qt4_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/kde3supportwidgets.cpp ${CMAKE_CURRENT_BINARY_DIR}/kde3supportwidgets.moc)


     qt4_add_resources(kde3supportwidgets_PART_SRCS kde3supportwidgets.qrc)

     kde4_add_plugin(kde3supportwidgets ${kde3supportwidgets_PART_SRCS})

     target_link_libraries(kde3supportwidgets ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS})
     if(NOT WIN32)
        set_target_properties(kde3supportwidgets PROPERTIES
                                     INSTALL_RPATH_USE_LINK_PATH TRUE
                                     SKIP_BUILD_RPATH TRUE
                                     BUILD_WITH_INSTALL_RPATH TRUE
                                     INSTALL_RPATH ${LIB_INSTALL_DIR}
                             )
     endif(NOT WIN32)

     install(TARGETS kde3supportwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )
  endif (QT_QT3SUPPORT_FOUND)
endif (QT_QTDESIGNER_FOUND)

########### install files ###############





