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
                    this, SLOT( toggleMinimizeRestore() ), 
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
        toggleMinimizeRestore();
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


void KSystemTray::toggleMinimizeRestore()
{
    if ( !parentWidget() )
	return;
    if ( !parentWidget()->isVisible() ) {
	parentWidget()->hide(); // KWin::setOnDesktop( parentWidget()->winId(), KWin::currentDesktop() );
#ifndef Q_WS_QWS //FIXME
	KWin::Info info = KWin::info( parentWidget()->winId() );
	parentWidget()->move( info.geometry.topLeft() );
#endif
	parentWidget()->show();
#ifndef Q_WS_QWS //FIXME
        KWin::deIconifyWindow( parentWidget()->winId() );
	KWin::setActiveWindow( parentWidget()->winId() );
#endif
    } else {
	parentWidget()->hide();
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
