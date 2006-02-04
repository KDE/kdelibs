INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(console_SRCS
main.cpp
console.cpp
\
)

KDE3_AUTOMOC(${console_SRCS})

SET( console_UI
jsconsole.ui
)

KDE3_ADD_UI_FILES(console_SRCS ${console_UI} )

KDE3_ADD_EXECUTABLE(console ${console_SRCS})

TARGET_LINK_LIBRARIES(console  ${QT_AND_KDECORE_LIBS} kjsembed )

INSTALL_TARGETS(/bin console )


########### install files ###############


KDE3_PLACEHOLDER()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I../kjsembed
#
#bin_PROGRAMS = console 
#
#console_SOURCES = jsconsole.ui console.qrc main.cpp console.cpp \ 
#                  kjs_object_model.cpp numberedtextview.cpp
#console_METASOURCES = AUTO
#console_LDFLAGS = $(all_libraries) 
#console_LDADD = -lkjsembed 
