kde4_header()

add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kimproxy_LIB_SRCS
kimproxy.cpp
)

kde4_automoc(${kimproxy_LIB_SRCS})

set( kimproxy_DCOP_SKEL_SRCS
kimproxyiface.h
)

kde4_add_dcop_skels(kimproxy_LIB_SRCS ${kimproxy_DCOP_SKEL_SRCS})

set( kimproxy_DCOP_STUB_SRCS
${CMAKE_SOURCE_DIR}/interfaces/kimproxy/interface/kimiface.h
)

kde4_add_dcop_stubs(kimproxy_LIB_SRCS ${kimproxy_DCOP_STUB_SRCS})

kde4_add_library(kimproxy SHARED ${kimproxy_LIB_SRCS})

target_link_libraries(kimproxy  ${QT_AND_KDECORE_LIBS} kio )

set_target_properties(kimproxy PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kimproxy )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = icons
#
#INCLUDES = $(all_includes)
#
#noinst_HEADERS = kimproxyiface.h
#
#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(all_includes)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kimproxy.pot
#
#kimproxyincludedir = $(includedir)
#kimproxyinclude_HEADERS = kimproxy.h kimproxyiface.h
#
#lib_LTLIBRARIES =  libkimproxy.la
#libkimproxy_la_LDFLAGS = -no-undefined $(all_libraries)
#libkimproxy_la_LIBADD = $(LIB_KIO)
#libkimproxy_la_SOURCES = kimproxy.cpp kimiface.stub kimproxyiface.skel
#
#kimiface_DIR = $(top_srcdir)/interfaces/kimproxy/interface
