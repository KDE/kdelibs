
include_directories( ${ASPELL_INCLUDE_DIR})


########### next target ###############

set(kspell_aspell_PART_SRCS kspell_aspellclient.cpp kspell_aspelldict.cpp )


add_library(kspell_aspell MODULE ${kspell_aspell_PART_SRCS})

target_link_libraries(kspell_aspell PRIVATE SonnetCore ${ASPELL_LIBRARIES})

install(TARGETS kspell_aspell  DESTINATION ${PLUGIN_INSTALL_DIR}/sonnet_clients/ )
