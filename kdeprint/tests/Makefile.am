INCLUDES = -I$(top_srcdir)/kdeprint -I$(top_srcdir) $(all_includes)

check_PROGRAMS = helpviewer drawdemo

helpviewer_SOURCES = helpwindow.cpp main.cpp richpage.cpp
helpviewer_METASOURCES = helpwindow.moc
helpviewer_LDADD = $(top_builddir)/kdeprint/libkdeprint.la
helpviewer_LDFLAGS = $(all_libraries)

drawdemo_SOURCES = drawdemo.cpp
drawdemo_METASOURCES = drawdemo.moc
drawdemo_LDADD = $(top_builddir)/kdeprint/libkdeprint.la
drawdemo_LDFLAGS = $(all_libraries)

EXTRA_DIST = home.xpm back.xpm forward.xpm
