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

#include "kaction.h"
#include "kshortcut.h"
#include "ksystemtray.h"
#include "kpopupmenu.h"
#include "kapplication.h"
#include "klocale.h"
#include <kwin.h>
#include <kwinmodule.h>
#include <kiconloader.h>

#include <qapplication.h>
#ifndef Q_WS_QWS
#include <X11/Xlib.h>
#ifndef KDE_USE_FINAL
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#endif
#undef FocusOut
#undef FocusIn
#undef KeyPress
#undef KeyRelease

extern Time qt_x_time;
#endif

class KSystemTrayPrivate
{
public:
    KSystemTrayPrivate()
    {
        actionCollection = 0;
    }

    ~KSystemTrayPrivate()
    {
        delete actionCollection;
    }

    KActionCollection* actionCollection;
};

KSystemTray::KSystemTray( QWidget* parent, const char* name )
    : QLabel( parent, name, WType_TopLevel )
{
    d = new KSystemTrayPrivate;
    d->actionCollection = new KActionCollection(this);

#ifndef Q_WS_QWS
    // FIXME(E): Talk with QWS
    KWin::setSystemTrayWindowFor( winId(), parent?parent->topLevelWidget()->winId(): qt_xrootwin() );
    setBackgroundMode(X11ParentRelative);
#endif
    hasQuit = 0;
    menu = new KPopupMenu( this );
    menu->insertTitle( kapp->miniIcon(), kapp->caption() );
    move( -1000, -1000 );
    KAction* quitAction = KStdAction::quit(this, SIGNAL(quitSelected()), d->actionCollection);

    if (parentWidget())
    {
        connect(quitAction, SIGNAL(activated()), parentWidget(), SLOT(close()));
        new KAction(i18n("Minimize"), KShortcut(), 
                    this, SLOT( minimizeRestoreAction() ),
                    d->actionCollection, "minimizeRestore");
    }
    else
    {
        connect(quitAction, SIGNAL(activated()), qApp, SLOT(closeAllWindows()));
    }
}

KSystemTray::~KSystemTray()
{
    delete d;
}


void KSystemTray::showEvent( QShowEvent * )
{
    if ( !hasQuit ) {
	menu->insertSeparator();
        KAction* action = d->actionCollection->action("minimizeRestore");

        if (action)
        {
            action->plug(menu);
        }

        action = d->actionCollection->action(KStdAction::name(KStdAction::Quit));
        action->plug(menu);
	hasQuit = 1;
    }
}

void KSystemTray::enterEvent( QEvent* )
{
#ifndef Q_WS_QWS
    // FIXME(E): Implement for Qt Embedded
    if ( !qApp->focusWidget() ) {
	XEvent ev;
	memset(&ev, 0, sizeof(ev));
	ev.xfocus.display = qt_xdisplay();
	ev.xfocus.type = XFocusIn;
	ev.xfocus.window = winId();
	ev.xfocus.mode = NotifyNormal;
	ev.xfocus.detail = NotifyAncestor;
	Time time = qt_x_time;
	qt_x_time = 1;
	qApp->x11ProcessEvent( &ev );
	qt_x_time = time;
    }
#endif
}

KPopupMenu* KSystemTray::contextMenu() const
{
    return menu;
}


void KSystemTray::mousePressEvent( QMouseEvent *e )
{
    if ( !rect().contains( e->pos() ) )
	return;

    switch ( e->button() ) {
    case LeftButton:
        activateOrHide();
	break;
    case MidButton:
	// fall through
    case RightButton:
	if ( parentWidget() ) {
            KAction* action = d->actionCollection->action("minimizeRestore");
	    if ( parentWidget()->isVisible() )
		action->setText( i18n("Minimize") );
	    else
		action->setText( i18n("Restore") );
	}
	contextMenuAboutToShow( menu );
	menu->popup( e->globalPos() );
	break;
    default:
	// nothing
	break;
    }
}

void KSystemTray::mouseReleaseEvent( QMouseEvent * )
{
}


void KSystemTray::contextMenuAboutToShow( KPopupMenu* )
{
}


// called from the popup menu - always do what the menu entry says,
// i.e. if the window is shown, no matter if active or not, the menu
// entry is "minimize", otherwise it's "restore"
void KSystemTray::minimizeRestoreAction()
{
    if ( parentWidget() ) {
        bool restore = !( parentWidget()->isVisible() );
	minimizeRestore( restore );
    }
}

// called when left-clicking the tray icon
// if the window is not the active one, show it if needed, and activate it
// (just like taskbar); otherwise hide it
void KSystemTray::activateOrHide()
{
    QWidget *pw = parentWidget();

    if ( !pw )
	return;

    KWin::Info info = KWin::info( pw->winId() );
    bool visible = (info.mappingState == NET::Visible);
    // hack for KWin's non-compliant WM_STATE handling
    visible = visible && ( info.desktop == KWin::currentDesktop());
    // SELI using !pw->isActiveWindow() should be enough here,
    // but it doesn't work - e.g. with kscd, the "active" window
    // is the widget docked in Kicker
    if ( visible && ( KWinModule().activeWindow() != pw->winId() )) // visible not active -> activate
    {
        KWin::setActiveWindow( pw->winId() );
        return;
    }
    minimizeRestore( !visible );
}

void KSystemTray::minimizeRestore( bool restore )
{
    QWidget* pw = parentWidget();
    if( !pw )
	return;
    if ( restore )
    {
#ifndef Q_WS_QWS //FIXME
	// TODO what to do with OnAllDesktops windows? (#32783)
	KWin::setOnDesktop( pw->winId(), KWin::currentDesktop());
	KWin::Info info = KWin::info( pw->winId() );
        pw->move( info.geometry.topLeft() ); // avoid placement policies
        pw->show();
	KWin::setActiveWindow( pw->winId() );
#endif
    } else {
	pw->hide();
    }
}

KActionCollection* KSystemTray::actionCollection()
{
    return d->actionCollection;
}

void KSystemTray::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksystemtray.moc"
#include "kdockwindow.moc"
