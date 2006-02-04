INCLUDES = $(all_includes) -I$(top_srcdir)/kjsembed/kjsembed

bin_PROGRAMS = kjscmd 

kjscmd_SOURCES = kjscmd.cpp kjscmd.qrc
kjscmd_LDFLAGS = $(all_libraries) 
kjscmd_LDADD = ../kjsembed/libkjsembed.la 

#man_MANS = kjscmd.1

#xdg_apps_DATA = kjscmd.desktop

