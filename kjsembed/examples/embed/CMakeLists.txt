kde4_header()



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


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#bin_PROGRAMS = embed 
#
#embed_SOURCES = embed.cpp test_object.cpp test_pointer.cpp
#embed_METASOURCES = AUTO 
#embed_LDFLAGS = $(all_libraries) 
#embed_LDADD = -lkjsembed
#
