#shamelessly lifted from dfaure's howto.. 
bin_PROGRAMS = testapp

testapp_SOURCES = app.cpp fileloader.cpp
testapp_LDADD = ../libklargeimages.la
testapp_LDFLAGS = $(all_libraries) $(KDE_RPATH)
 
AM_CPPFLAGS = $(all_includes)
 
METASOURCES = AUTO 
INCLUDES = -I$(top_srcdir)/style-workshop/largeimagelib $(all_includes)
