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

#include "config.h"
#include "kaction.h"
#include "kshortcut.h"
#include "ksystemtray.h"
#include "kpopupmenu.h"
#include "kapplication.h"
#include "klocale.h"

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <kwin.h> // schroder
#include <kwinmodule.h> // schroder
#endif

#include <kiconloader.h>
#include <kconfig.h>

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <qxembed.h> // schroder
#endif

#include <qapplication.h>

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <X11/Xlib.h> // schroder
#ifndef KDE_USE_FINAL
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#endif
#undef FocusOut
#undef FocusIn
#undef KeyPress
#undef KeyRelease

extern Time qt_x_time;
#endif // Q_WS_X11 && ! K_WS_QTONLY

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
    bool on_all_desktops; // valid only when the parent widget was hidden
};

KSystemTray::KSystemTray( QWidget* parent, const char* name )
    : QLabel( parent, name, WType_TopLevel )
{
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    QXEmbed::initialize();
#endif
    
    d = new KSystemTrayPrivate;
    d->actionCollection = new KActionCollection(this);

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
//#ifndef Q_WS_QWS
    // FIXME(E): Talk with QWS
    KWin::setSystemTrayWindowFor( winId(), parent?parent->topLevelWidget()->winId(): qt_xrootwin() );
    setBackgroundMode(X11ParentRelative);
    setBackgroundOrigin(WindowOrigin);
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
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
	KWin::WindowInfo info = KWin::windowInfo( parentWidget()->winId());
	d->on_all_desktops = info.onAllDesktops();
#endif
    }
    else
    {
        connect(quitAction, SIGNAL(activated()), qApp, SLOT(closeAllWindows()));
	d->on_all_desktops = false;
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

        if (action)
        {
            action->plug(menu);
        }

	hasQuit = 1;
    }
}

// KDE4 remove
void KSystemTray::enterEvent( QEvent* e )
{
#if QT_VERSION < 0x030200
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
//#ifndef Q_WS_QWS
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
#endif
    QLabel::enterEvent( e );
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
		action->setText( i18n("&Minimize") );
	    else
		action->setText( i18n("&Restore") );
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

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    KWin::WindowInfo info1 = KWin::windowInfo( pw->winId(), NET::XAWMState | NET::WMState );
    // mapped = visible (but possibly obscured)
    bool mapped = (info1.mappingState() == NET::Visible) && !info1.isMinimized();
//    - not mapped -> show, raise, focus
//    - mapped
//        - obscured -> raise, focus
//        - not obscured -> hide
    if( !mapped )
        minimizeRestore( true );
    else
    {
        KWinModule module;
        for( QValueList< WId >::ConstIterator it = module.stackingOrder().fromLast();
             it != module.stackingOrder().end() && (*it) != pw->winId();
             --it )
        {
            KWin::WindowInfo info2 = KWin::windowInfo( *it,
                NET::WMGeometry | NET::XAWMState | NET::WMState | NET::WMWindowType );
            if( info2.mappingState() != NET::Visible )
                continue; // not visible on current desktop -> ignore
            if( !info2.geometry().intersects( pw->geometry()))
                continue; // not obscuring the window -> ignore
            if( !info1.hasState( NET::KeepAbove ) && info2.hasState( NET::KeepAbove ))
                continue; // obscured by window kept above -> ignore
            NET::WindowType type = info2.windowType( NET::NormalMask | NET::DesktopMask
                | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
                | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );
            if( type == NET::Dock || type == NET::TopMenu )
                continue; // obscured by dock or topmenu -> ignore
            pw->raise();
            KWin::activateWindow( pw->winId());
            return;
        }
        minimizeRestore( false ); // hide
    }
#endif
}

void KSystemTray::minimizeRestore( bool restore )
{
    QWidget* pw = parentWidget();
    if( !pw )
	return;
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    KWin::WindowInfo info = KWin::windowInfo( pw->winId(), NET::WMGeometry | NET::WMDesktop );
    if ( restore )
    {
//#ifndef Q_WS_QWS //FIXME
//#if defined Q_WS_X11 && ! defined K_WS_QTONLY
	if( d->on_all_desktops )
	    KWin::setOnAllDesktops( pw->winId(), true );
	else
	    KWin::setOnDesktop( pw->winId(), KWin::currentDesktop());
        pw->move( info.geometry().topLeft() ); // avoid placement policies
        pw->show();
        pw->raise();
	KWin::activateWindow( pw->winId() );
    } else {
	d->on_all_desktops = info.onAllDesktops();
	pw->hide();
    }
#endif
}

KActionCollection* KSystemTray::actionCollection()
{
    return d->actionCollection;
}
    
QPixmap KSystemTray::loadIcon( const QString &icon, KInstance *instance )
{
    KConfig *appCfg = kapp->config();
    KConfigGroupSaver configSaver(appCfg, "System Tray");
    int iconWidth = appCfg->readNumEntry("systrayIconWidth", 22);
    return instance->iconLoader()->loadIcon( icon, KIcon::Panel, iconWidth );
}

void KSystemTray::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksystemtray.moc"
#include "kdockwindow.moc"
