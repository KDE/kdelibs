INCLUDES = -I$(srcdir)/.. $(all_includes)

lib_LTLIBRARIES = libkmediaplayer.la

libkmediaplayer_la_SOURCES = player.cpp playerdcopobject.skel view.cpp
libkmediaplayer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkmediaplayer_la_LIBADD = $(LIB_KPARTS)
libkmediaplayer_la_METASOURCES = AUTO

kmediaplayerinclude_HEADERS = player.h playerdcopobject.h view.h
kmediaplayerincludedir = $(includedir)/kmediaplayer

kde_servicetypes_DATA = kmediaplayerengine.desktop kmediaplayer.desktop
