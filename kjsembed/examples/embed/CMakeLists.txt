kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(embed_SRCS
embed.cpp
test_object.cpp
test_pointer.cpp
)

kde4_automoc(${embed_SRCS})

kde4_add_executable(embed ${embed_SRCS})

target_link_libraries(embed  ${QT_AND_KDECORE_LIBS} kjsembed )

install_targets(/bin embed )


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
