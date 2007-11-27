/*

  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

*/
#include <QtGui/QWidget>
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
