include_directories( ${KDE4_KDEUI_INCLUDES} ) # for kwallet.h

########### next target ###############

set(kwalletbackend_LIB_SRCS
   blockcipher.cc
   blowfish.cc
   cbc.cc
   sha1.cc
   kwalletentry.cc
   kwalletbackend.cc
   )


kde4_add_library(kwalletbackend SHARED ${kwalletbackend_LIB_SRCS})

target_link_libraries(kwalletbackend ${KDE4_KDEUI_LIBS} )

# link with advapi32 on windows
if(WIN32)
    target_link_libraries(kwalletbackend advapi32)
endif(WIN32)

set_target_properties(kwalletbackend PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kwalletbackend ${INSTALL_TARGETS_DEFAULT_ARGS})

