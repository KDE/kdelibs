/* This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

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
 * kwin.cpp Part of the KDE project.
 */

#include "kwin.h"
#include <qwmatrix.h>
#include <qbitmap.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#undef WithdrawnState
#undef IconicState
#undef NormalState

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


static bool atoms = FALSE;

static Atom net_number_of_desktops;
static Atom net_current_desktop;
static Atom net_active_window;
static Atom net_kde_docking_window_for;

extern Atom qt_wm_state;

static void createAtoms() {
    if (!atoms){
	net_number_of_desktops = XInternAtom(qt_xdisplay(), "_NET_NUMBER_OF_DESKTOPS", False);
	net_current_desktop = XInternAtom(qt_xdisplay(), "_NET_CURRENT_DESKTOP", False);
	net_active_window = XInternAtom(qt_xdisplay(), "_NET_ACTIVE_WINDOW", False);
	net_kde_docking_window_for = XInternAtom(qt_xdisplay(), "_NET_KDE_DOCKING_WINDOW_FOR", False);
	atoms = True;
    }
}

/*
  Sends a client message to the ROOT window.
 */
static void sendClientMessage(Window w, Atom a, long x){
  XEvent ev;
  long mask;

  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = CurrentTime;
  mask = SubstructureRedirectMask;
  XSendEvent(qt_xdisplay(), qt_xrootwin(), False, mask, &ev);
}



int KWin::numberOfDesktops()
{
    createAtoms();
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    int result = 0;
    if ( XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), net_number_of_desktops, 0, 1,
			     FALSE, XA_CARDINAL, &type, &format,
			     &length, &after, &data ) == Success ) {
	if ( data ) {
	    result = ( (int*)data)[0];
	}
	XFree( data );
    }
    return result;
}

void KWin::setNumberOfDesktops(int num)
{
    createAtoms();
    sendClientMessage( qt_xrootwin(), net_number_of_desktops, num );
}

int KWin::currentDesktop()
{
    createAtoms();
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    int result = 0;
    if ( XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), net_current_desktop, 0, 1,
			     FALSE, XA_CARDINAL, &type, &format,
			     &length, &after, &data ) == Success ) {
	if ( data ) {
	    result = ( (int*)data)[0];
	}
	XFree( data );
    }
    return result;
}

void KWin::setCurrentDesktop( int desktop )
{
    createAtoms();
    sendClientMessage( qt_xrootwin(), net_current_desktop, desktop );
}


WId KWin::activeWindow()
{
    createAtoms();
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    WId result = 0;
    if ( XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), net_active_window, 0, 1,
			     FALSE, XA_WINDOW, &type, &format,
			     &length, &after, &data ) == Success ) {
	if ( data ) {
	    result = ( (WId*)data)[0];
	    XFree( data );
	}
    }
    return result;
}


void KWin::setActiveWindow( WId win)
{
    createAtoms();
    sendClientMessage( win, net_active_window, 0);
}


void KWin::setDockWindow(WId dockWin, WId forWin ){
    createAtoms();
    XChangeProperty(qt_xdisplay(), dockWin, net_kde_docking_window_for, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *)&forWin, 1);
}


bool KWin::isDockWindow( WId dockWin, WId *forWin )
{
    createAtoms();
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    bool result = FALSE;
    if ( forWin )
	*forWin = 0;
    if ( XGetWindowProperty( qt_xdisplay(), dockWin, net_kde_docking_window_for, 0, 1,
			     FALSE, XA_WINDOW, &type, &format,
			     &length, &after, &data ) == Success ) {
	if ( data ) {
	    result = TRUE;
	    if ( forWin )
		*forWin = ( (WId*)data)[0];
	    XFree( data );
	}
    }
    return result;
}

KWin::WindowState KWin::windowState( WId win )
{
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    WindowState result = KWin::WithdrawnState;
    int r = XGetWindowProperty( qt_xdisplay(), win, qt_wm_state, 0, 2,
				FALSE, AnyPropertyType, &type, &format,
				&length, &after, &data );
    if ( r == Success && data && format == 32 ) {
	Q_UINT32 *wstate = (Q_UINT32*)data;
	result = (WindowState) *wstate;
	XFree( (char *)data );
    }
    return result;
}
