
if(KDE4_BUILD_TESTS)

include_directories( ${KDE4_KPARTS_INCLUDES} )


########### next target ###############

set(kpartstest_SRCS
example.cpp
parts.cpp
)

kde4_automoc(${kpartstest_SRCS})

kde4_add_executable(kpartstest ${kpartstest_SRCS})

target_link_libraries(kpartstest  ${KDE4_KDECORE_LIBS} kparts )


########### next target ###############

set(normalktmtest_SRCS
normalktm.cpp
parts.cpp
)

kde4_automoc(${normalktmtest_SRCS})

kde4_add_executable(normalktmtest ${normalktmtest_SRCS})

target_link_libraries(normalktmtest  ${KDE4_KDECORE_LIBS} kparts )


########### next target ###############

set(ghostviewtest_SRCS
ghostview.cpp
)

kde4_automoc(${ghostviewtest_SRCS})

kde4_add_executable(ghostviewtest ${ghostviewtest_SRCS})

target_link_libraries(ghostviewtest  ${KDE4_KDECORE_LIBS} kparts )


########### next target ###############

set(spellcheckplugin_PART_SRCS plugin_spellcheck.cpp )

kde4_automoc(${spellcheckplugin_PART_SRCS})

kde4_add_plugin(spellcheckplugin WITH_PREFIX ${spellcheckplugin_PART_SRCS})

target_link_libraries(spellcheckplugin  ${KDE4_KDECORE_LIBS} kparts )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} spellcheckplugin )
install_targets(${PLUGIN_INSTALL_DIR} spellcheckplugin )


########### next target ###############

set(notepadpart_PART_SRCS notepadpart.cpp )

kde4_automoc(${notepadpart_PART_SRCS})

kde4_add_plugin(notepadpart WITH_PREFIX ${notepadpart_PART_SRCS})

target_link_libraries(notepadpart  ${KDE4_KDECORE_LIBS} kparts )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} notepadpart )
install_targets(${PLUGIN_INSTALL_DIR} notepadpart )


########### next target ###############


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/notepadpart/kpartplugins FILES plugin_foobar.rc plugin_spellcheck.rc )
install_files( ${DATA_INSTALL_DIR}/notepadpart FILES notepadpart.rc )
install_files( ${DATA_INSTALL_DIR}/ghostviewtest FILES ghostviewtest_shell.rc )
install_files( ${DATA_INSTALL_DIR}/kpartstest FILES kpartstest_shell.rc )
install_files( ${DATA_INSTALL_DIR}/kpartstestpart FILES kpartstest_part1.rc )

endif(KDE4_BUILD_TESTS)




