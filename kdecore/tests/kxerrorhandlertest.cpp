#include <qwidget.h>
#include <X11/Xlib.h>
#include <iostream>
using namespace std;

#include <kxerrorhandler.h>

int handler1( Display*, XErrorEvent* e )
    {
    cout << "ERR1:" << e->resourceid << ":" << (int)e->error_code << ":" << (int)e->request_code << ":" << e->serial << endl;
    return 1;
    }

bool handler3( int request, int error_code, unsigned long resourceid )
    {
    cout << "ERR3:" << resourceid << ":" << error_code << ":" << request << endl;
    return true;
    }
    
int main()
    {
    Display* dpy = XOpenDisplay( NULL );
    XSetWindowAttributes attrs;
    Window w = XCreateWindow( dpy, DefaultRootWindow( dpy ), 0, 0, 100, 100, 0, CopyFromParent, CopyFromParent,
        CopyFromParent, 0, &attrs );
    cout << w << ":" << XNextRequest( dpy ) << endl;
    XMapWindow( dpy, w );
    ++w;
//    XSetInputFocus( dpy, w, RevertToParent, CurrentTime );
        {
        KXErrorHandler handle1( handler1, dpy );
        cout << w << ":" << XNextRequest( dpy ) << endl;
        XMapWindow( dpy, w );
        XWindowAttributes attr;
            {
            KXErrorHandler handle2( dpy );
            XGetWindowAttributes(dpy, w, &attr);
                {
                KXErrorHandler handle3( handler3, dpy );
                XSetInputFocus( dpy, w, RevertToParent, CurrentTime );
                cout << "WAS3:" << handle3.error( /*false*/ true ) << endl;
                }
            cout << "WAS2:" << handle2.error( false ) << endl;
            }
//        XSync( dpy, False );
        cout << "WAS1:" << handle1.error( false ) << endl;
        }
    for(;;)
        {
        XEvent ev;
        XNextEvent( dpy, &ev );
        }
    XCloseDisplay( dpy );
    }
