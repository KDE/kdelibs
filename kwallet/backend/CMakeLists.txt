
########### next target ###############

set(kwalletbackend_LIB_SRCS
blockcipher.cc
blowfish.cc
cbc.cc
sha1.cc
kwalletentry.cc
kwalletbackend.cc
)

kde4_automoc(${kwalletbackend_LIB_SRCS})

kde4_add_library(kwalletbackend SHARED ${kwalletbackend_LIB_SRCS})

target_link_libraries(kwalletbackend  ${KDE4_KDECORE_LIBS} kdecore )

set_target_properties(kwalletbackend PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kwalletbackend  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############





