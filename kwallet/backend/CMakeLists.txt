kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kwalletbackend_LIB_SRCS
blockcipher.cc
blowfish.cc
cbc.cc
sha1.cc
kwalletentry.cc
kwalletbackend.cc
)

kde4_automoc(${kwalletbackend_LIB_SRCS})

kde4_add_library(kwalletbackend SHARED ${kwalletbackend_LIB_SRCS})

target_link_libraries(kwalletbackend  ${QT_AND_KDECORE_LIBS} kdecore )

set_target_properties(kwalletbackend PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kwalletbackend )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = -I$(srcdir)/../client $(all_includes)
#
#lib_LTLIBRARIES = libkwalletbackend.la
#
#libkwalletbackend_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0 -no-undefined
#libkwalletbackend_la_LIBADD = $(LIB_QT) ../../kdecore/libkdecore.la
#libkwalletbackend_la_SOURCES = blockcipher.cc		\
#			       blowfish.cc		\
#			       cbc.cc			\
#			       sha1.cc			\
#			       kwalletentry.cc		\
#			       kwalletbackend.cc
#
#
#libkwalletbackend_la_METASOURCES = AUTO
#
#noinst_HEADERS = blowfishtables.h	\
#		 cbc.h			\
#		 sha1.h			\
#		 blockcipher.h		\
#		 kwalletentry.h		\
#		 kwalletbackend.h	\
#		 blowfish.h
#
