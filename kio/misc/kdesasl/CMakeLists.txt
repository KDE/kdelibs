kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdesasl_LIB_SRCS
kdesasl.cpp
)

kde4_automoc(${kdesasl_LIB_SRCS})

kde4_add_library(kdesasl SHARED ${kdesasl_LIB_SRCS})

target_link_libraries(kdesasl  ${QT_AND_KDECORE_LIBS} )

set_target_properties(kdesasl PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(/lib kdesasl )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=$(all_includes)
#
#lib_LTLIBRARIES = libkdesasl.la
#METASOURCES     = AUTO
#
#kdesaslincludedir = $(includedir)/kio
#kdesaslinclude_HEADERS = kdesasl.h
#
#libkdesasl_la_SOURCES = kdesasl.cpp
#libkdesasl_la_LDFLAGS = $(all_libraries) -version-info 3:0:2 -no-undefined
#libkdesasl_la_LIBADD = $(LIB_KDECORE)
#
