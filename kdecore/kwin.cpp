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
#include "kapp.h"
#include <qwmatrix.h>
#include <qbitmap.h>
#include <qwhatsthis.h>
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


static bool atoms_created = FALSE;

static Atom wm_protocols;
static Atom net_number_of_desktops;
static Atom net_current_desktop;
static Atom net_active_window;
static Atom net_wm_context_help;
static Atom net_kde_docking_window_for;
static Atom kwm_dock_window;

extern Atom qt_wm_state;

static void createAtoms() {
    if (!atoms_created){
	net_number_of_desktops = XInternAtom(qt_xdisplay(), "_NET_NUMBER_OF_DESKTOPS", False);
	net_current_desktop = XInternAtom(qt_xdisplay(), "_NET_CURRENT_DESKTOP", False);
	net_active_window = XInternAtom(qt_xdisplay(), "_NET_ACTIVE_WINDOW", False);
	net_kde_docking_window_for = XInternAtom(qt_xdisplay(), "_NET_KDE_DOCKING_WINDOW_FOR", False);
	kwm_dock_window = XInternAtom(qt_xdisplay(), "KWM_DOCKWINDOW", False);

	const int max = 20;
	Atom* atoms[max];
	const char* names[max];
	Atom atoms_return[max];
	int n = 0;
	
	atoms[n] = &wm_protocols;
	names[n++] = "WM_PROTOCOLS";
	
	atoms[n] = &net_number_of_desktops;
	names[n++] = "_NET_NUMBER_OF_DESKTOPS";

	atoms[n] = &net_current_desktop;
	names[n++] = "_NET_CURRENT_DESKTOP";

	atoms[n] = &net_active_window;
	names[n++] = "_NET_ACTIVE_WINDOW";

	atoms[n] = &net_wm_context_help;
	names[n++] = "_NET_WM_CONTEXT_HELP";

	atoms[n] = &net_kde_docking_window_for;
	names[n++] = "_NET_KDE_DOCKING_WINDOW_FOR";

	// we need a const_cast for the shitty X API
	XInternAtoms( qt_xdisplay(), const_cast<char**>(names), n, FALSE, atoms_return );
	for (int i = 0; i < n; i++ )
	    *atoms[i] = atoms_return[i];

	atoms_created = True;
    }
}

/*
  Send a client message to window w
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
  mask = 0L;
  if (w == qt_xrootwin())
    mask = SubstructureRedirectMask;        /* magic! */
  XSendEvent(qt_xdisplay(), w, False, mask, &ev);
}

/*
  Sends a client message to the ROOT window.
 */
static void sendClientMessageToRoot(Window w, Atom a, long x){
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
    sendClientMessageToRoot( qt_xrootwin(), net_number_of_desktops, num );
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
    sendClientMessageToRoot( qt_xrootwin(), net_current_desktop, desktop );
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
    sendClientMessageToRoot( win, net_active_window, 0);
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

    bool netDocking =  XGetWindowProperty( qt_xdisplay(), dockWin, net_kde_docking_window_for, 0, 1,
					   FALSE, XA_WINDOW, &type, &format,
					   &length, &after, &data ) == Success;
     
    if ( netDocking ) {
	if ( data ) {
	    result = TRUE;
	    if ( forWin && netDocking )
	       *forWin = ( (WId*)data)[0];  
	    XFree( data );
	}
    }

    // check for KDE1 docking
    if ( !result )
    {
       bool kwmDocking =  XGetWindowProperty( qt_xdisplay(), dockWin, kwm_dock_window, 0, 1,
					      FALSE, XA_WINDOW, &type, &format,
					      &length, &after, &data ) == Success;
       if ( kwmDocking ) {
	  if ( data ) {
	     result = TRUE;
	     XFree( data );
	  }
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



class ContextWidget : public QWidget
{
public:
    ContextWidget()
	: QWidget(0,0)
    {
       kapp->installX11EventFilter( this );
      QWhatsThis::enterWhatsThisMode();
      QCursor c = *QApplication::overrideCursor();
      QWhatsThis::leaveWhatsThisMode();
      XGrabPointer( qt_xdisplay(), qt_xrootwin(), TRUE,
		    (uint)( ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask | EnterWindowMask |
			    LeaveWindowMask ),
		    GrabModeAsync, GrabModeAsync,
		    None, c.handle(), CurrentTime );
      qApp->enter_loop();
    }


    bool x11Event( XEvent * ev)
    {
	if ( ev->type == ButtonPress && ev->xbutton.button == Button1 ) {
	    XUngrabPointer( qt_xdisplay(), ev->xbutton.time );
	    Window root;
	    Window child = qt_xrootwin();
	    int root_x, root_y, lx, ly;
	    uint state;
	    Window w;
	    do {
		w = child;
		XQueryPointer( qt_xdisplay(), w, &root, &child,
			       &root_x, &root_y, &lx, &ly, &state );
	    } while  ( child != None && child != w );
	
	    sendClientMessage(w, wm_protocols, net_wm_context_help);
	    XEvent e = *ev;
	    e.xbutton.window = w;
	    e.xbutton.subwindow = w;
	    e.xbutton.x = lx;
	    e.xbutton.y = ly;
	    XSendEvent( qt_xdisplay(), w, TRUE, ButtonPressMask, &e );
	    qApp->exit_loop();
	    return TRUE;
	}
	return FALSE;
    }
};

void KWin::invokeContextHelp()
{
    ContextWidget w;
}
