include_directories(
    ${CMAKE_SOURCE_DIR}/kross
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

# the scriptmanager module
set(krossmodulescriptmanager_PART_SRCS scriptmanager.cpp)
kde4_automoc(krossmodulescriptmanager ${krossmodulescriptmanager_PART_SRCS})
kde4_add_plugin(krossmodulescriptmanager ${krossmodulescriptmanager_PART_SRCS})

target_link_libraries(krossmodulescriptmanager ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} knewstuff krosscore)
install(TARGETS krossmodulescriptmanager DESTINATION ${PLUGIN_INSTALL_DIR})

# the forms module
set(krossmoduleforms_PART_SRCS form.cpp)
kde4_automoc(${krossmoduleforms_PART_SRCS})
kde4_add_plugin(krossmoduleforms ${krossmoduleforms_PART_SRCS})

target_link_libraries(krossmoduleforms ${QT_QTUITOOLS_LIBRARY} ${QT_QTDESIGNER_LIBRARY} ${KDE4_KDECORE_LIBS} krosscore)
install(TARGETS krossmoduleforms DESTINATION ${PLUGIN_INSTALL_DIR})
