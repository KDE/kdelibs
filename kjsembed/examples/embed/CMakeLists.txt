INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(embed_SRCS
embed.cpp
test_object.cpp
test_pointer.cpp
)

KDE3_AUTOMOC(${embed_SRCS})

KDE3_ADD_EXECUTABLE(embed ${embed_SRCS})

TARGET_LINK_LIBRARIES(embed  ${QT_AND_KDECORE_LIBS} kjsembed )

INSTALL_TARGETS(/bin embed )


########### install files ###############


KDE3_PLACEHOLDER()



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
