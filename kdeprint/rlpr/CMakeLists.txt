
include_directories( ${CMAKE_SOURCE_DIR}/kdeprint/management )


########### next target ###############

set(kdeprint_rlpr_PART_SRCS
kmrlprfactory.cpp
kmrlprmanager.cpp
krlprprinterimpl.cpp
kmrlpruimanager.cpp
kmwrlpr.cpp
kmproprlpr.cpp
kmproxywidget.cpp
kmconfigproxy.cpp
)

kde4_automoc(${kdeprint_rlpr_PART_SRCS})

kde4_add_plugin(kdeprint_rlpr ${kdeprint_rlpr_PART_SRCS})

target_link_libraries(kdeprint_rlpr  ${KDE4_KDECORE_LIBS} kdeprint_management )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdeprint_rlpr )
install_targets(${PLUGIN_INSTALL_DIR} kdeprint_rlpr )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeprint/plugins FILES rlpr.print )




