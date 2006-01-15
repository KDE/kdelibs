kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kntlm_LIB_SRCS
kntlm.cpp
des.cpp
)

kde4_automoc(${kntlm_LIB_SRCS})

kde4_add_library(kntlm SHARED ${kntlm_LIB_SRCS})

target_link_libraries(kntlm  ${QT_AND_KDECORE_LIBS} )

set_target_properties(kntlm PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(/lib kntlm )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=$(all_includes)
#
#lib_LTLIBRARIES = libkntlm.la
#METASOURCES     = AUTO
#
#kntlmincludedir = $(includedir)/kio
#kntlminclude_HEADERS = kntlm.h
#
#libkntlm_la_SOURCES = kntlm.cpp des.cpp
#libkntlm_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -no-undefined
#libkntlm_la_LIBADD = $(LIB_KDECORE)
#
