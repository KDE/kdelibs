kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kwalletclient_LIB_SRCS
kwallet.cc
)

kde4_automoc(${kwalletclient_LIB_SRCS})

set( kwalletclient_DCOP_SKEL_SRCS
kwallet.h
)

kde4_add_dcop_skels(kwalletclient_LIB_SRCS ${kwalletclient_DCOP_SKEL_SRCS})

kde4_add_library(kwalletclient SHARED ${kwalletclient_LIB_SRCS})

target_link_libraries(kwalletclient  ${QT_AND_KDECORE_LIBS} )

set_target_properties(kwalletclient PROPERTIES VERSION 1.1.0 SOVERSION 1 )
install_targets(/lib kwalletclient )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES= -I$(srcdir) $(all_includes)
#
#lib_LTLIBRARIES = libkwalletclient.la
#
#libkwalletclient_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:1 -no-undefined
#libkwalletclient_la_LIBADD = $(LIB_KDECORE)
#libkwalletclient_la_SOURCES =   kwallet.skel \
#				kwallet.cc
#
#libkwalletclient_la_METASOURCES = AUTO
#
#include_HEADERS = kwallet.h kwallettypes.h
#
