kde4_header()



########### next target ###############

set(designer_SRCS
main.cpp
)

kde4_automoc(${designer_SRCS})

kde4_add_executable(designer ${designer_SRCS})

target_link_libraries(designer  ${KDE4_KDECORE_LIBS} kjsembed QtDesignerComponents_debug )

install(TARGETS designer DESTINATION bin)


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#bin_PROGRAMS = designer 
#
#designer_SOURCES = main.cpp 
#designer_METASOURCES = AUTO 
#designer_LDFLAGS = $(all_libraries) 
#designer_LDADD = -lkjsembed -lQtDesignerComponents_debug 
#
