/* This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

    $Id$

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

#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

#include <qapplication.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qwhatsthis.h>
#include <qcstring.h>

#ifndef Q_WS_QWS
#include "kwin.h"
#include "kapplication.h"

#include <kglobal.h>
#include <kiconloader.h>

#include <kdatastream.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kstartupinfo.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "netwm.h"

static bool atoms_created = FALSE;
extern Atom qt_wm_protocols;
extern Atom qt_wm_state;

// Fix for --enable-final. This gets undefined at the end of this file.
#ifndef None
#define	None	0L
#endif

Atom net_wm_context_help;
static Atom kde_wm_change_state;
void kwin_net_create_atoms() {
    if (!atoms_created){
	const int max = 20;
	Atom* atoms[max];
	const char* names[max];
	Atom atoms_return[max];
	int n = 0;
	
	atoms[n] = &net_wm_context_help;
	names[n++] = "_NET_WM_CONTEXT_HELP";

	atoms[n] = &kde_wm_change_state;
	names[n++] = "_KDE_WM_CHANGE_STATE";
	
	// we need a const_cast for the shitty X API
	XInternAtoms( qt_xdisplay(), const_cast<char**>(names), n, FALSE, atoms_return );
	for (int i = 0; i < n; i++ )
	    *atoms[i] = atoms_return[i];

	atoms_created = True;
    }
}

/*
  Sends a client message to the ROOT window.
 */
static void sendClientMessageToRoot(Window w, Atom a, long x, long y = 0, long z = 0 ){
  XEvent ev;
  long mask;

  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = y;
  ev.xclient.data.l[2] = z;
  mask = SubstructureRedirectMask;
  XSendEvent(qt_xdisplay(), qt_xrootwin(), False, mask, &ev);
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

class ContextWidget : public QWidget
{
public:
    ContextWidget()
	: QWidget(0,0)
    {
	kwin_net_create_atoms();
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
	
	    ::sendClientMessage(w, qt_wm_protocols, net_wm_context_help);
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

void KWin::setSystemTrayWindowFor( WId trayWin, WId forWin )
{
    NETWinInfo info( qt_xdisplay(), trayWin, qt_xrootwin(), 0 );
    if ( !forWin )
	forWin = qt_xrootwin();
    info.setKDESystemTrayWinFor( forWin );
}
void KWin::setActiveWindow( WId win)
{
    NETRootInfo info( qt_xdisplay(), 0 );
    info.setActiveWindow( win );
}

KWin::Info KWin::info( WId win )
{
    Info w;
    NETWinInfo inf( qt_xdisplay(), win, qt_xrootwin(),
		    NET::WMState |
		    NET::WMStrut |
		    NET::WMWindowType |
		    NET::WMName |
		    NET::WMVisibleName |
		    NET::WMDesktop |
		    NET::WMPid |
		    NET::WMKDEFrameStrut |
		    NET::XAWMState
		    );

    w.win = win;
    w.state = inf.state();
    w.mappingState = inf.mappingState();
    w.strut = inf.strut();
    w.windowType = inf.windowType();
    if ( inf.name() ) {
	w.name = QString::fromUtf8( inf.name() );
    } else {
	char* c = 0;
	if ( XFetchName( qt_xdisplay(), win, &c ) != 0 ) {
	    w.name = QString::fromLocal8Bit( c );
	    XFree( c );
	}
    }
    if ( inf.visibleName() )
	w.visibleName = QString::fromUtf8( inf.visibleName() );
    else
	w.visibleName = w.name;

    w.desktop = inf.desktop();
    w.onAllDesktops = inf.desktop() == NETWinInfo::OnAllDesktops;
    w.pid = inf.pid();
    NETRect frame, geom;
    inf.kdeGeometry( frame, geom );
    w.geometry.setRect( geom.pos.x, geom.pos.y, geom.size.width, geom.size.height );
    w.frameGeometry.setRect( frame.pos.x, frame.pos.y, frame.size.width, frame.size.height );
    return w;
}

QPixmap KWin::icon( WId win, int width, int height, bool scale )
{
    QPixmap result;
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), NET::WMIcon );
    NETIcon ni = info.icon( width, height );
    if ( ni.data && ni.size.width > 0 && ni.size.height > 0 ) {
	QImage img( (uchar*) ni.data, (int) ni.size.width, (int) ni.size.height, 32, 0, 0, QImage::IgnoreEndian );
	img.setAlphaBuffer( TRUE );
	if ( scale && width > 0 && height > 0 &&img.size() != QSize( width, height ) && !img.isNull() )
	    img = img.smoothScale( width, height );
	if ( !img.isNull() )
	    result.convertFromImage( img );
	return result;
    }

    Pixmap p = None;
    Pixmap p_mask = None;

    XWMHints *hints = XGetWMHints(qt_xdisplay(), win );
    if (hints && (hints->flags & IconPixmapHint)){
	p = hints->icon_pixmap;
    }
    if (hints && (hints->flags & IconMaskHint)){
	p_mask = hints->icon_mask;
    }
    if (hints)
	XFree((char*)hints);

    if (p != None){
	Window root;
	int x, y;
	unsigned int w = 0;
	unsigned int h = 0;
	unsigned int border_w, depth;
	XGetGeometry(qt_xdisplay(), p, &root,
		     &x, &y, &w, &h, &border_w, &depth);
	if (w > 0 && h > 0){
	    QPixmap pm(w, h, depth);
	    XCopyArea(qt_xdisplay(), p, pm.handle(),
		      qt_xget_temp_gc(qt_xscreen(), depth==1),
		      0, 0, w, h, 0, 0);
	    if (p_mask != None){
		QBitmap bm(w, h);
		XCopyArea(qt_xdisplay(), p_mask, bm.handle(),
			  qt_xget_temp_gc(qt_xscreen(), true),
			  0, 0, w, h, 0, 0);
		pm.setMask(bm);
	    }
	    if ( scale && width > 0 && height > 0 && !pm.isNull() && 
		 ( (int) w != width || (int) h != height) ){
		result.convertFromImage( pm.convertToImage().smoothScale( width, height ) );
	    } else {
		result = pm;
	    }
	}
    }
	
    // Try to load the icon from the classhint if the app didn't specify
    // its own:
    if( result.isNull() ) {
	int iconWidth;
			
	    // Since width can be any arbitrary size, but the icons cannot,
	    // take the nearest value for best results (ignoring 22 pixel
	    // icons as they don't exist for apps):
	if( width < 24 )
	    iconWidth = 16;
	else if( width < 40 )
	    iconWidth = 32;
	else
	    iconWidth = 48;
			
	XClassHint	hint;
	if( XGetClassHint( qt_xdisplay(), win, &hint ) ) {
	    QString className = hint.res_class;

	    QPixmap pm = KGlobal::instance()->iconLoader()->loadIcon( className.lower(), KIcon::Small, iconWidth,
								      KIcon::DefaultState, 0, true );
	    if( scale && !pm.isNull() )
		result.convertFromImage( pm.convertToImage().smoothScale( width, height ) );
	    else
		result = pm;

	}

	// If the icon is still a null pixmap, load the 'xapp' icon
	// as a last resort:
	if ( result.isNull() ) {
	    QPixmap pm = KGlobal::instance()->iconLoader()->loadIcon(  "xapp", KIcon::Small, iconWidth,
								       KIcon::DefaultState, 0, true );
	    if( scale && !pm.isNull() )
		result.convertFromImage( pm.convertToImage().smoothScale( width, height ) );
	    else
		result = pm;
	}
    }
    return result;
}

void KWin::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon )
{
    if ( icon.isNull() )
	return;
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), 0 );
    QImage img = icon.convertToImage().convertDepth( 32 );
    NETIcon ni;
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, true );
    if ( miniIcon.isNull() )
	return;
    img = miniIcon.convertToImage().convertDepth( 32 );
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, false );
}

void KWin::setType( WId win, NET::WindowType windowType )
{
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), 0 );
    info.setWindowType( windowType );
}

void KWin::setState( WId win, unsigned long state )
{
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), NET::WMState );
    info.setState( state, state );
}

void KWin::clearState( WId win, unsigned long state )
{
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), NET::WMState );
    info.setState( 0, state );
}

void KWin::setOnAllDesktops( WId win, bool b )
{
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), NET::WMDesktop );
    if ( b )
	info.setDesktop( NETWinInfo::OnAllDesktops );
    else if ( info.desktop()  == NETWinInfo::OnAllDesktops ) {
	NETRootInfo rinfo( qt_xdisplay(), NET::CurrentDesktop );
	info.setDesktop( rinfo.currentDesktop() );
    }
}

void KWin::setOnDesktop( WId win, int desktop )
{
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), NET::WMDesktop );
    info.setDesktop( desktop );
}


QString KWin::Info::visibleNameWithState() const
{
    QString s = visibleName;
    if ( isIconified() ) {
	s.prepend('(');
	s.append(')');
    }
    return s;
}


void KWin::setStrut( WId win, int left, int right, int top, int bottom )
{
    NETWinInfo info( qt_xdisplay(), win, qt_xrootwin(), 0 );
    NETStrut strut;
    strut.left = left;
    strut.right = right;
    strut.top = top;
    strut.bottom = bottom;
    info.setStrut( strut );
}

int KWin::currentDesktop()
{
    if (!qt_xdisplay())
      return 0;
    NETRootInfo info( qt_xdisplay(), NET::CurrentDesktop );
    return info.currentDesktop();
}

int KWin::numberOfDesktops()
{
    if (!qt_xdisplay())
      return 0;
    NETRootInfo info( qt_xdisplay(), NET::NumberOfDesktops );
    return info.numberOfDesktops();
}

void KWin::setCurrentDesktop( int desktop )
{
    NETRootInfo info( qt_xdisplay(), NET::CurrentDesktop );
    info.setCurrentDesktop( desktop );
}


void KWin::iconifyWindow( WId win, bool animation)
{
    if ( !animation )
	sendClientMessageToRoot( win, kde_wm_change_state, IconicState, 1 );
    XIconifyWindow( qt_xdisplay(), win, qt_xscreen() );
}


void KWin::deIconifyWindow( WId win, bool animation )
{
    if ( !animation )
	sendClientMessageToRoot( win, kde_wm_change_state, NormalState, 1 );
    XMapWindow( qt_xdisplay(), win );
}

void KWin::appStarted()
{
    KStartupInfo::appStarted();
}

// Fix for --enable-final. This gets defined at the top of this file.
#undef	None
#endif
