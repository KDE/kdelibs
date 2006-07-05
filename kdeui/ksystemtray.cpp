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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "kaboutdata.h"
#include "kaction.h"
#include "kinstance.h"
#include "klocale.h"
#include "kmenu.h"
#include "kmessagebox.h"
#include "kshortcut.h"
#include "ksystemtray.h"
#include "kactioncollection.h"
#include "kstdaction.h"

#ifdef Q_WS_X11
#include <kwin.h>
#include <kwinmodule.h>
#include <qxembed.h>
#include <QX11Info>
#endif

#include <kconfig.h>
#include <kiconloader.h>

#include <QApplication>
#include <QMouseEvent>

class KSystemTrayPrivate
{
public:
    KSystemTrayPrivate(KSystemTray* trayIcon, QWidget* parent)
    {
        actionCollection = new KActionCollection( trayIcon );
        hasQuit = false;
        onAllDesktops = false;
        menu = new KMenu;
        window = parent;
    }

    ~KSystemTrayPrivate()
    {
        delete actionCollection;
        delete menu;
    }

    KActionCollection* actionCollection;
    KMenu* menu;
    QWidget* window;
    bool onAllDesktops; // valid only when the parent widget was hidden
    bool hasQuit;
};

KSystemTray::KSystemTray( QWidget* parent )
    : QSystemTrayIcon( parent )
{
    d = new KSystemTrayPrivate( this, parent );

    d->menu = new KMenu( parent );
    d->menu->addTitle( qApp->windowIcon(), KInstance::caption() );
    d->menu->setTitle( KGlobal::instance()->aboutData()->programName() );
    connect( d->menu, SIGNAL( aboutToShow() ), this, SLOT( contextMenuAboutToShow() ) );
    setContextMenu( d->menu );

    KStdAction::quit( this, SLOT( maybeQuit() ), d->actionCollection );

    if ( parent )
    {
        KAction *action = new KAction( i18n( "Minimize" ), d->actionCollection, "minimizeRestore");
        connect( action, SIGNAL( triggered( bool ) ), this, SLOT( minimizeRestoreAction() ) );

#ifdef Q_WS_X11
        KWin::WindowInfo info = KWin::windowInfo( parent->winId() );
        d->onAllDesktops = info.onAllDesktops();
#else
        d->onAllDesktops = false;
#endif
    }
    else
    {
        d->onAllDesktops = false;
    }

    connect( this, SIGNAL( activated( int ) ), this, SLOT( activateOrHide( int ) ) );
}

KSystemTray::~KSystemTray()
{
    delete d;
}

void KSystemTray::showEvent( QShowEvent * )
{
    if ( d->hasQuit )
    {
        return;
    }

    d->menu->addSeparator();
    KAction* action = d->actionCollection->action( "minimizeRestore" );

    if ( action )
    {
        d->menu->addAction( action );
    }

    action = d->actionCollection->action( KStdAction::name( KStdAction::Quit ) );

    if ( action )
    {
        d->menu->addAction( action );
    }

    d->hasQuit = true;
}

void KSystemTray::contextMenuAboutToShow( )
{
    if ( d->window )
    {
        KAction* action = d->actionCollection->action("minimizeRestore");
        if ( d->window->isVisible() )
        {
            action->setText( i18n("&Minimize") );
        }
        else
        {
            action->setText( i18n("&Restore") );
        }
    }
}

// called from the popup menu - always do what the menu entry says,
// i.e. if the window is shown, no matter if active or not, the menu
// entry is "minimize", otherwise it's "restore"
void KSystemTray::minimizeRestoreAction()
{
    if ( d->window )
    {
        bool restore = !( d->window->isVisible() );
        minimizeRestore( restore );
    }
}

void KSystemTray::maybeQuit()
{
    QString caption = KInstance::caption();
    QString query = i18n("<qt>Are you sure you want to quit <b>%1</b>?</qt>",
                         caption);
    if (KMessageBox::warningContinueCancel(d->window, query,
                                     i18n("Confirm Quit From System Tray"),
                                     KStdGuiItem::quit(),
                                     QString("systemtrayquit%1")
                                            .arg(caption)) !=
        KMessageBox::Continue)
    {
        return;
    }

    emit quitSelected();
    qApp->quit();
}

// if the window is not the active one, show it if needed, and activate it
// (just like taskbar); otherwise hide it
void KSystemTray::activateOrHide( int reasonCalled )
{
    if ( reasonCalled != QSystemTrayIcon::Trigger )
    {
        return;
    }

    QWidget *pw = d->window;
    if ( !pw )
    {
        return;
    }

#ifdef Q_WS_X11
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
        QListIterator< WId > it (module.stackingOrder());
        it.toBack();
        while( it.hasPrevious() )
        {
            WId id = it.previous();
            if( id == pw->winId() )
                break;
            KWin::WindowInfo info2 = KWin::windowInfo( id,
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
    QWidget* pw = d->window;
    if( !pw )
	return;
#ifdef Q_WS_X11
    KWin::WindowInfo info = KWin::windowInfo( pw->winId(), NET::WMGeometry | NET::WMDesktop );
    if ( restore )
    {
	if( d->onAllDesktops )
	    KWin::setOnAllDesktops( pw->winId(), true );
	else
	    KWin::setCurrentDesktop( info.desktop() );
        pw->move( info.geometry().topLeft() ); // avoid placement policies
        pw->show();
        pw->raise();
	KWin::activateWindow( pw->winId() );
    } else {
	d->onAllDesktops = info.onAllDesktops();
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
    KConfigGroup cg(instance->config(), "System Tray");
    int iconWidth = cg.readEntry("systrayIconWidth", 22);
    return instance->iconLoader()->loadIcon( icon, K3Icon::Panel, iconWidth );
}

#include "ksystemtray.moc"

