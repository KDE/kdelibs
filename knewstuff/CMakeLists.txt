
project(knewstuff)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(knewstuff_LIB_SRCS
   engine.cpp
   entry.cpp
   downloaddialog.cpp
   uploaddialog.cpp
   providerdialog.cpp
   provider.cpp
   knewstuff.cpp
   knewstuffgeneric.cpp
   knewstuffbutton.cpp
   knewstuffsecure.cpp
   security.cpp
   )

kde4_automoc(${knewstuff_LIB_SRCS})

kde4_add_library(knewstuff SHARED ${knewstuff_LIB_SRCS})

target_link_libraries(knewstuff  ${KDE4_KDECORE_LIBS} kio )

set_target_properties(knewstuff PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS knewstuff  DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

set(khotnewstuff_SRCS khotnewstuff.cpp )

kde4_automoc(${khotnewstuff_SRCS})

kde4_add_executable(khotnewstuff ${khotnewstuff_SRCS})

target_link_libraries(khotnewstuff  ${KDE4_KDECORE_LIBS} knewstuff)

install(TARGETS khotnewstuff DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

if(KDE4_BUILD_TESTS)

   set(testnewstuff_SRCS testnewstuff.cpp )
   
   kde4_automoc(${testnewstuff_SRCS})
   
   kde4_add_executable(testnewstuff ${testnewstuff_SRCS})
   
   target_link_libraries(testnewstuff  ${KDE4_KDECORE_LIBS} knewstuff)
   
   
   ########### next target ###############
   
   set(ghns_SRCS ghns.cpp )
   
   kde4_automoc(${ghns_SRCS})
   
   kde4_add_executable(ghns ${ghns_SRCS})
   
   target_link_libraries(ghns  ${KDE4_KDECORE_LIBS} knewstuff)

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES khotnewstuffrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES downloaddialog.h engine.h entry.h knewstuffgeneric.h knewstuff.h providerdialog.h provider.h uploaddialog.h knewstuffbutton.h knewstuffsecure.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/knewstuff )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




