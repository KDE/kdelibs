

########### next target ###############

set(kwalletclient_LIB_SRCS
kwallet.cc
)

kde4_automoc(${kwalletclient_LIB_SRCS})


kde4_add_library(kwalletclient SHARED ${kwalletclient_LIB_SRCS})

target_link_libraries(kwalletclient  ${KDE4_KDECORE_LIBS} )

set_target_properties(kwalletclient PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install(TARGETS kwalletclient  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES kwallet.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )

