/* This file is part of the KDE libraries

   Copyright (C) 1999 Mattias Ettrich (ettrich@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


/*
 * kipc.cpp: Send a message to all KDE apps.
 *
 * $Id$
 */

#include <qwindowdefs.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <kipc.h>


int KIPC::dropError(Display *, XErrorEvent *)
{
    return 0;
}


long KIPC::getSimpleProperty(Window w, Atom a)
{
    Atom real_type;
    int format;
    unsigned long n, extra, *p, res;
    int status;

    res = 0;
    status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, 1L, False, a, 
            &real_type, &format, &n, &extra, (unsigned char **) &p);
    if ((status == Success) && (n == 1) && (format == 32))
	res = p[0];
    XFree((char *) p);
    return res;
}


void KIPC::sendMessage(Atom msg, Window w, int data)
{
    XEvent ev;
    ev.xclient.type = ClientMessage;
    ev.xclient.display = qt_xdisplay();
    ev.xclient.window = w;
    ev.xclient.message_type = msg;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = data;
    XSendEvent(qt_xdisplay(), w, False, 0L, &ev);
}


void KIPC::sendMessage(const char *msg, Window w, int data)
{
    Atom a = XInternAtom(qt_xdisplay(), msg, False);
    sendMessage(a, w, data);
}


void KIPC::sendMessageAll(Atom msg, int data)
{
    XEvent ev;
    unsigned int i, nrootwins;
    Window dw1, dw2, *rootwins;
    int (*defaultHandler)(Display *, XErrorEvent *);
    Display *dpy = qt_xdisplay();
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    defaultHandler = XSetErrorHandler(&KIPC::dropError);
    
    XQueryTree(dpy, root, &dw1, &dw2, &rootwins, &nrootwins);
    
    long result;
    Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
    for (i = 0; i < nrootwins; i++) {
        result = KIPC::getSimpleProperty(rootwins[i], a);
        if (result) {
            ev.xclient.type = ClientMessage;
            ev.xclient.display = dpy;
            ev.xclient.window = rootwins[i];
            ev.xclient.message_type = msg;
            ev.xclient.format = 32;
	    ev.xclient.data.l[0] = data;
            XSendEvent(dpy, rootwins[i], False, 0L, &ev);
        }
    }

    XFlush(dpy);
    XSetErrorHandler(defaultHandler);
    XFree((char *) rootwins);
}


void KIPC::sendMessageAll(const char *msg, int data)
{
    Atom a = XInternAtom(qt_xdisplay(), msg, False);
    sendMessageAll(a, data);
}
