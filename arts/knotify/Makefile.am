
INCLUDES= $(all_includes)

####### Files

bin_PROGRAMS	= knotify knotifyclient
knotify_SOURCES	= knotify.cpp
knotify_LDADD	= $(LIB_KDEUI) $(LIB_KDECORE)
knotify_LDFLAGS	= $(all_libraries) $(KDE_RPATH)
METASOURCES = knotify.moc

knotifyclient_SOURCES	= knotifyclient.cpp
knotifyclient_LDADD	= $(LIB_KDECORE)
knotifyclient_LDFLAGS	= $(all_libraries) $(KDE_RPATH)

noinst_HEADERS	=  knotify.h

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(knotify_SOURCES) && mv messages.po $(podir)/knotify.pot

edit:
	xemacs $(knotify_SOURCES) $(noinst_HEADERS) &
