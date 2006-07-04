


########### next target ###############

set(designer_SRCS
main.cpp
)

kde4_automoc(${designer_SRCS})

kde4_add_executable(designer ${designer_SRCS})

target_link_libraries(designer  ${KDE4_KDECORE_LIBS} kjsembed QtDesignerComponents_debug )

install(TARGETS designer DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############





