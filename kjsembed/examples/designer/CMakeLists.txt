kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(designer_SRCS
main.cpp
)

kde4_automoc(${designer_SRCS})

kde4_add_executable(designer ${designer_SRCS})

target_link_libraries(designer  ${QT_AND_KDECORE_LIBS} kjsembed QtDesignerComponents_debug )

install_targets(/bin designer )


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
