kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(klegacystyle_LIB_SRCS
klegacystyle.cpp
plugin.cpp
)

kde4_automoc(${klegacystyle_LIB_SRCS})

kde4_add_library(klegacystyle SHARED ${klegacystyle_LIB_SRCS})

target_link_libraries(klegacystyle  ${QT_AND_KDECORE_LIBS} )

set_target_properties(klegacystyle PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib klegacystyle )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(all_includes) 
#
#noinst_HEADERS = klegacystyle.h
#lib_LTLIBRARIES = klegacystyle.la
#klegacystyle_la_LDFLAGS = $(all_libraries) -module  -avoid-version -no-undefined 
#klegacystyle_la_LIBADD = $(LIB_KDECORE) 
#klegacystyle_la_SOURCES = klegacystyle.cpp plugin.cpp
#klegacystyle_la_METASOURCES = AUTO
