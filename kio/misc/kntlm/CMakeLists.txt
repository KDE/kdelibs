kde4_header()

########### next target ###############

set(kntlm_LIB_SRCS kntlm.cpp des.cpp )

kde4_automoc(${kntlm_LIB_SRCS})

kde4_add_library(kntlm SHARED ${kntlm_LIB_SRCS})

target_link_libraries(kntlm  ${KDE4_KDECORE_LIBS} )

set_target_properties(kntlm PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kntlm )


########### install files ###############
install_files( /include/kio FILES kntlm.h )


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
