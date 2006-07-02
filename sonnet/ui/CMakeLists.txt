


########### next target ###############

set(kspell2_LIB_SRCS
 dialog.cpp
 ${kspell2base_STAT_SRCS}
 configwidget.cpp
 highlighter.cpp
 configdialog.cpp
)

kde4_automoc(${kspell2_LIB_SRCS})

kde4_add_ui_files(kspell2_LIB_SRCS kspell2ui.ui configui.ui)

kde4_add_library(kspell2 SHARED ${kspell2_LIB_SRCS})

target_link_libraries(kspell2  ${KDE4_KDECORE_LIBS} kdeui)

set_target_properties(kspell2 PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} kspell2 )


########### next target ###############


########### install files ###############
install_files( /include/kspell2 FILES dialog.h highlighter.h configdialog.h   configwidget.h )





