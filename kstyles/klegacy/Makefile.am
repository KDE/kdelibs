INCLUDES= $(all_includes) 

noinst_HEADERS = klegacystyle.h
lib_LTLIBRARIES = klegacystyle.la
klegacystyle_la_LDFLAGS = $(all_libraries) -module  -avoid-version -no-undefined 
klegacystyle_la_LIBADD = $(LIB_KDECORE) 
klegacystyle_la_SOURCES = klegacystyle.cpp plugin.cpp
klegacystyle_la_METASOURCES = AUTO
