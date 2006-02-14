
INCLUDES= $(all_includes) $(akode_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

####### Files

#kde_module_LTLIBRARIES = knotify.la
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = knotify.la

knotify_la_SOURCES	= main.cpp knotify.cpp knotify.skel knotifyplugin.cpp knotifyconfig.cpp \
 	notifybysound.cpp notifybypopup.cpp notifybylogfile.cpp notifybytaskbar.cpp notifybyexecute.cpp
knotify_la_LIBADD	= $(LIB_KDEUI) $(akode_libs)
knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version
knotify_la_METASOURCES = AUTO

noinst_HEADERS	=  knotify.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = knotify.desktop

kdedatadir = $(kde_datadir)/kde
kdedata_DATA = kde.notifyrc


messages:
	$(XGETTEXT) knotify.cpp -o $(podir)/knotify.pot

