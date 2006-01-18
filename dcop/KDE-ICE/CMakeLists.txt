kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#AM_CFLAGS = -DICE_t -DTRANS_SERVER -DTRANS_CLIENT -DUNIXCONN -DANSICPP
#INCLUDES = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#noinst_LTLIBRARIES = libkICE.la
#
#noinst_HEADERS =  ICE.h \
#	ICEconn.h \
#	ICElib.h \
#	ICElibint.h \
#	ICEmsg.h \
#	ICEproto.h \
#	ICEutil.h \
#	Xtrans.h \
#	Xtransint.h \
#	Xfuncproto.h \
#	globals.h 
#
#libkICE_la_SOURCES = accept.c \
#       authutil.c \
#       connect.c \
#       error.c \
#       getauth.c \
#       iceauth.c \
#       listen.c \
#       listenwk.c \
#       locking.c \
#       misc.c \
#       ping.c \
#       process.c \
#       protosetup.c \
#       register.c \
#       replywait.c \
#       setauth.c \
#       shutdown.c \
#       watch.c \
#       transport.c \
#       globals.c
