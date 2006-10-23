project(kjsembed-embed-example)

include_directories( ${KDE4_KDEUI_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/kjsembed ${CMAKE_SOURCE_DIR}/kjsembed/kjsembed )


########### next target ###############

set(embed_SRCS
  embed.cpp
  test_object.cpp
  test_pointer.cpp
)

set( embed_QRC
  embed.qrc
)

qt4_add_resources(embed_SRCS ${embed_QRC} )

kde4_automoc(${embed_SRCS})

kde4_add_executable(embed ${embed_SRCS})

target_link_libraries(embed  ${KDE4_KDECORE_LIBS} kjsembed )

#install(TARGETS embed DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############





