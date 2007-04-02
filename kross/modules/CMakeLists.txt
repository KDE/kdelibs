include_directories(
    ${CMAKE_SOURCE_DIR}/kross
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kfile
)

############################################
# the forms module

set(krossmoduleforms_SRCS form.cpp)
kde4_automoc(${krossmoduleforms_SRCS})
kde4_add_plugin(krossmoduleforms ${krossmoduleforms_SRCS})

target_link_libraries(krossmoduleforms ${QT_QTUITOOLS_LIBRARY} ${QT_QTDESIGNER_LIBRARY} ${KDE4_KPARTS_LIBS} ${KDE4_KFILE_LIBS} krosscore)
install(TARGETS krossmoduleforms DESTINATION ${PLUGIN_INSTALL_DIR})

############################################
# the scriptmanager module

set(krossmodulescriptmanager_SRCS scriptmanager.cpp scriptmanageradd.cpp)
kde4_automoc(krossmodulescriptmanager ${krossmodulescriptmanager_SRCS})
kde4_add_plugin(krossmodulescriptmanager ${krossmodulescriptmanager_SRCS})

#target_link_libraries(krossmodulescriptmanager ${KDE4_KFILE_LIBS} knewstuff krosscore)
target_link_libraries(krossmodulescriptmanager ${KDE4_KFILE_LIBS} krosscore)
install(TARGETS krossmodulescriptmanager DESTINATION ${PLUGIN_INSTALL_DIR})
