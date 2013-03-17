
include_directories( ${ASPELL_INCLUDE_DIR})


########### next target ###############

set(kspell_aspell_PART_SRCS kspell_aspellclient.cpp kspell_aspelldict.cpp )


add_library(kspell_aspell MODULE ${kspell_aspell_PART_SRCS})

target_link_libraries(kspell_aspell  ${QT_QTCORE_LIBRARY} sonnetcore ${ASPELL_LIBRARIES} )

install(TARGETS kspell_aspell  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/sonnet_clients/ )
