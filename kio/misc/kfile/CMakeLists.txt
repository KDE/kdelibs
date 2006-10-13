
########### next target ###############

set(kfile_SRCS fileprops.cpp )

kde4_automoc(kfile ${kfile_SRCS})

kde4_add_executable(kfile NOGUI ${kfile_SRCS})

target_link_libraries(kfile  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kfile DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############





