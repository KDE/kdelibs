INCLUDES = -I$(top_srcdir)/kdeprint $(all_includes)

noinst_PROGRAMS = helpviewer drawdemo

helpviewer_SOURCES = helpwindow.cpp main.cpp richpage.cpp
helpviewer_METASOURCES = helpwindow.moc
helpviewer_LDADD = ../kdeprint/libkdeprint.la
helpviewer_LDFLAGS = $(all_libraries)

drawdemo_SOURCES = drawdemo.cpp
drawdemo_METASOURCES = drawdemo.moc
drawdemo_LDADD = ../kdeprint/libkdeprint.la
drawdemo_LDFLAGS = $(all_libraries)

EXTRA_DIST = home.xpm back.xpm forward.xpm
