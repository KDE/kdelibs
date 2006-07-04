

########### next target ###############

set(kwalletclient_LIB_SRCS
kwallet.cc
)

kde4_automoc(${kwalletclient_LIB_SRCS})


kde4_add_library(kwalletclient SHARED ${kwalletclient_LIB_SRCS})

target_link_libraries(kwalletclient  ${KDE4_KDECORE_LIBS} )

set_target_properties(kwalletclient PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kwalletclient  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES kwallet.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )





