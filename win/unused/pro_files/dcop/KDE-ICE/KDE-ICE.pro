TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )

DEFINES += ICE_t TRANS_SERVER TRANS_CLIENT SOCKCONN TCPCONN ANSICPP

TARGET = kdeice$$KDEBUG$(KDE_VER)

system( bash kmoc )

DEF_FILE = kde-ice.def

CONFIG += no_precompile_header console
CONFIG	-= dll 
CONFIG	+= staticlib

SOURCES	+= iceauth.c \
			process.c \
			accept.c \
			authutil.c \
			connect.c \
			error.c \
			getauth.c \
			globals.c \
			listen.c \
			listenwk.c \
			locking.c \
			misc.c \
			ping.c \
			protosetup.c \
			register.c \
			replywait.c \
			setauth.c \
			shutdown.c \
			transport.c \
			watch.c

