
include_directories( ${HSPELL_INCLUDE_DIR}  )


########### next target ###############

set(kspell_hspell_PART_SRCS kspell_hspellclient.cpp kspell_hspelldict.cpp )


add_library(kspell_hspell MODULE ${kspell_hspell_PART_SRCS})

target_link_libraries(kspell_hspell  ${QT_QTCORE_LIBRARY} sonnetcore ${HSPELL_LIBRARIES} ${ZLIB_LIBRARY} )

install(TARGETS kspell_hspell  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/sonnet_clients/ )

