INCLUDES = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libkderesources.la

libkderesources_la_SOURCES = resource.cpp \
             resourcefactory.cpp resourcemanageriface.skel resourcemanageriface.stub resourcemanagerimpl.cpp \
	     resourceconfigwidget.cpp resourceconfigdlg.cpp resourceselectdialog.cpp \
	     resourcesconfigpage.cpp

libkderesources_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
libkderesources_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI)

kresincludedir = $(includedir)/kresources
kresinclude_HEADERS = resource.h resourceconfigwidget.h \
	 resourcefactory.h resourcemanageriface.h resourcemanagerimpl.h resourcemanager.h \
         resourceselectdialog.h resourceconfigdlg.h resourcesconfigpage.h 
        
# noinst_HEADERS = 
# plugin.h 
	
