
INCLUDES= $(all_includes) -I$(kde_includes)/arts

####### Files

bin_PROGRAMS	= knotify knotifyclient
knotify_SOURCES	= knotify.cpp
knotify_LDADD	= $(kde_libraries)/libsoundserver_idl.la \
                  $(kde_libraries)/libqtmcop.la $(LIB_KDEUI) $(LIB_KDECORE)
knotify_LDFLAGS	= $(all_libraries) $(KDE_RPATH)
METASOURCES = knotify.moc

knotifyclient_SOURCES	= knotifyclient.cpp
knotifyclient_LDADD	= $(LIB_KDECORE)
knotifyclient_LDFLAGS	= $(all_libraries) $(KDE_RPATH)

noinst_HEADERS	=  knotify.h

messages:
	$(XGETTEXT) $(knotify_SOURCES) -o $(podir)/knotify.pot

edit:
	xemacs $(knotify_SOURCES) $(noinst_HEADERS) &
