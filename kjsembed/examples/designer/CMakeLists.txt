INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(designer_SRCS
main.cpp
)

KDE3_AUTOMOC(${designer_SRCS})

KDE3_ADD_EXECUTABLE(designer ${designer_SRCS})

TARGET_LINK_LIBRARIES(designer  ${QT_AND_KDECORE_LIBS} kjsembed QtDesignerComponents_debug )

INSTALL_TARGETS(/bin designer )


########### install files ###############


KDE3_PLACEHOLDER()



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
