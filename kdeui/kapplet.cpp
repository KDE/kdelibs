#include "qxembed.h"
#include "kapplet.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>

static Atom kde_applet = 0;
static Atom kde_applet_command = 0;



KApplet::KApplet( QWidget* parent, const char* name  )
    : QWidget( parent, name )
{
    if ( !kde_applet ) {
	kde_applet =  XInternAtom(qt_xdisplay(), "KDE_APPLET", FALSE);
	kde_applet_command = XInternAtom(qt_xdisplay(), "KDE_APPLET_COMMAND", FALSE);
    }
}

KApplet::~KApplet()
{
}

void KApplet::setup( int& argc, char ** argv )
{
    int actual_format;
    Atom actual_type;
    unsigned long nitems, bytesafter;
    unsigned long *data;

    Orientation orientation = Horizontal;
    WId applet_panner = 0;
    int ideal_width = 42;
    int ideal_height = 42;
    
    XChangeProperty( qt_xdisplay(), winId(),
		     kde_applet_command, XA_STRING, 8, PropModeReplace,
		     (unsigned char *)argv[0], qstrlen( argv[0] ) );
    
    if(XGetWindowProperty (qt_xdisplay(), qt_xrootwin(), kde_applet, 0L, 4L,
			   False, kde_applet, &actual_type,
			   &actual_format, &nitems,
			   &bytesafter, (unsigned char **)&data)==Success) {
	applet_panner = data[0];
	ideal_width = data[1];
	ideal_height = data[2];
	orientation = (Orientation) data[3];
	XFree(( unsigned char* ) data );
    }


    init ( orientation, ideal_width, ideal_height );
    
    if ( applet_panner != 0 ) {
	XEvent ev;
	long mask;
	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = applet_panner;
	ev.xclient.message_type = kde_applet;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = winId();
	ev.xclient.data.l[1] = 0;
	ev.xclient.data.l[2] = 0;
	ev.xclient.data.l[3] = 0;
	ev.xclient.data.l[4] = 0;
	mask = 0L;
	XSendEvent(qt_xdisplay(), applet_panner, False, mask, &ev);
	XFlush( qt_xdisplay() );
	int count = 0;
	while ( !XCheckTypedWindowEvent( qt_xdisplay(), 
					winId(), 
					ReparentNotify,
					 &ev ) ) {
	    count++;
	    if ( count > 100 ) {
		exit(1);
	    }
	    usleep( 50000 );
	}
	XPutBackEvent( qt_xdisplay(), &ev );
    }
    
    
    /*

    int myargc = argc;
    int i, j;

    j = 1;
    for ( i=1; i<myargc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( strcmp(arg,"-horizontal") == 0 ) {
	    orientation = Horizontal;
	    i++;
	} else if ( strcmp(arg,"-vertical") == 0 ) {
	    orientation = Vertical;
	    i++;
	} else if ( strcmp(arg,"-width") == 0 && i < myargc-1 ) {
	    QCString s = argv[++i];
	    ideal_width = s.toInt();
	} else if ( strcmp(arg,"-height") == 0 && i < myargc-1 ) {
	    QCString s = argv[++i];
	    ideal_height = s.toInt();
	} else
	    argv[j++] = argv[i];
    }
    argc = j;

    init ( orientation, ideal_width, ideal_height );
    QXEmbed::processClientCmdline( this, argc, argv );
    
    */
}




void KApplet::init( Orientation /* orientation */, int width, int height  )
{
    resize(width, height );
}


QSize KApplet::sizeHint() const
{
    return size();
}


#include "kapplet.moc"
