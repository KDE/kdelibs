


########### next target ###############

set(kspell2_LIB_SRCS
 dialog.cpp
 ${sonnetbase_STAT_SRCS}
 configwidget.cpp
 highlighter.cpp
 configdialog.cpp
)

kde4_automoc(${kspell2_LIB_SRCS})

kde4_add_ui_files(kspell2_LIB_SRCS kspell2ui.ui configui.ui)

kde4_add_library(kspell2 SHARED ${kspell2_LIB_SRCS})

target_link_libraries(kspell2  ${KDE4_KDECORE_LIBS} kdeui)

set_target_properties(kspell2 PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kspell2  DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############


########### install files ###############
install( FILES dialog.h highlighter.h configdialog.h   configwidget.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/sonnet )





