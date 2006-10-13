
########### next target ###############

set(kntlm_LIB_SRCS kntlm.cpp des.cpp )

kde4_automoc(kntlm ${kntlm_LIB_SRCS})

kde4_add_library(kntlm SHARED ${kntlm_LIB_SRCS})

target_link_libraries(kntlm  ${KDE4_KDECORE_LIBS} )

set_target_properties(kntlm PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kntlm  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES kntlm.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kio )





