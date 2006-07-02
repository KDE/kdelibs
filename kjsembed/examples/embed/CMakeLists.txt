


########### next target ###############

set(embed_SRCS
embed.cpp
test_object.cpp
test_pointer.cpp
)

kde4_automoc(${embed_SRCS})

kde4_add_executable(embed ${embed_SRCS})

target_link_libraries(embed  ${KDE4_KDECORE_LIBS} kjsembed )

install(TARGETS embed DESTINATION bin)


########### install files ###############





