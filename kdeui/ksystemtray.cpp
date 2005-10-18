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
#include "kmessagebox.h"
#include "kshortcut.h"
#include "ksystemtray.h"
#include "kmenu.h"
#include <kapplication.h>
#include "klocale.h"
#include "kaboutdata.h"

#ifdef Q_WS_X11
#include <kwin.h> 
#include <kwinmodule.h> 
#include <qxembed.h> 
#include <QX11Info>
#endif

#include <kiconloader.h>
#include <kconfig.h>

#include <qapplication.h>
#include <qevent.h>

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
    : QLabel( parent, name, Qt::WType_TopLevel )
{
#ifdef Q_WS_X11
    #warning KDE4 porting: reinstate when the new QXEmbed is ready
    //QXEmbed::initialize();
#endif
    
    d = new KSystemTrayPrivate;
    d->actionCollection = new KActionCollection(this);

#ifdef Q_WS_X11
    KWin::setSystemTrayWindowFor( winId(), parent?parent->topLevelWidget()->winId(): QX11Info::appRootWindow() );
#endif
    setBackgroundMode(Qt::X11ParentRelative);
    setBackgroundOrigin(WindowOrigin);
    hasQuit = 0;
    menu = new KMenu( this );
    menu->addTitle( qApp->windowIcon(), kapp->caption() );
    move( -1000, -1000 );
    KStdAction::quit(this, SLOT(maybeQuit()), d->actionCollection);

    if (parentWidget())
    {
        new KAction(i18n("Minimize"), KShortcut(),
                    this, SLOT( minimizeRestoreAction() ),
                    d->actionCollection, "minimizeRestore");
#ifdef Q_WS_X11
	KWin::WindowInfo info = KWin::windowInfo( parentWidget()->winId());
	d->on_all_desktops = info.onAllDesktops();
#else
	d->on_all_desktops = false;
#endif
    }
    else
    {
        d->on_all_desktops = false;
    }
    setCaption( KGlobal::instance()->aboutData()->programName());
}

KSystemTray::~KSystemTray()
{
    delete d;
}


void KSystemTray::showEvent( QShowEvent * )
{
    if ( !hasQuit ) {
        menu->addSeparator();
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


KMenu* KSystemTray::contextMenu() const
{
    return menu;
}


void KSystemTray::mousePressEvent( QMouseEvent *e )
{
    if ( !rect().contains( e->pos() ) )
	return;

    switch ( e->button() ) {
    case Qt::LeftButton:
        toggleActive();
	break;
    case Qt::MidButton:
	// fall through
    case Qt::RightButton:
	if ( parentWidget() ) {
            KAction* action = d->actionCollection->action("minimizeRestore");
	    if ( parentWidget()->isVisible() )
		action->setText( i18n("&Minimize") );
	    else
		action->setText( i18n("&Restore") );
	}
	contextMenuAboutToShow( menu );
	menu->exec( e->globalPos() );
	break;
    default:
	// nothing
	break;
    }
}

void KSystemTray::mouseReleaseEvent( QMouseEvent * )
{
}


void KSystemTray::contextMenuAboutToShow( KMenu* )
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

void KSystemTray::maybeQuit()
{
    QString query = i18n("<qt>Are you sure you want to quit <b>%1</b>?</qt>")
                        .arg(kapp->caption());
    if (KMessageBox::warningContinueCancel(this, query,
                                     i18n("Confirm Quit From System Tray"),
                                     KStdGuiItem::quit(),
                                     QString("systemtrayquit%1")
                                            .arg(kapp->caption())) !=
        KMessageBox::Continue)
    {
        return;
    }

    emit quitSelected();

    // KDE4: stop closing the parent widget? it results in complex application code
    //       instead make applications connect to the quitSelected() signal

    if (parentWidget())
    {
        parentWidget()->close();
    }
    else
    {
        qApp->closeAllWindows();
    }
}

void KSystemTray::toggleActive()
{
    activateOrHide();
}

void KSystemTray::setActive()
{
    minimizeRestore( true );
}

void KSystemTray::setInactive()
{
    minimizeRestore( false );
}

// called when left-clicking the tray icon
// if the window is not the active one, show it if needed, and activate it
// (just like taskbar); otherwise hide it
void KSystemTray::activateOrHide()
{
    QWidget *pw = parentWidget();

    if ( !pw )
	return;

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
    QWidget* pw = parentWidget();
    if( !pw )
	return;
#ifdef Q_WS_X11
    KWin::WindowInfo info = KWin::windowInfo( pw->winId(), NET::WMGeometry | NET::WMDesktop );
    if ( restore )
    {
	if( d->on_all_desktops )
	    KWin::setOnAllDesktops( pw->winId(), true );
	else
	    KWin::setCurrentDesktop( info.desktop() );
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

void KSystemTray::setPixmap( const QPixmap& p )
{
    QLabel::setPixmap( p );
#ifdef Q_WS_X11
    KWin::setIcons( winId(), p, QPixmap());
#endif
}

void KSystemTray::setCaption( const QString& s )
{
    QLabel::setCaption( s );
}

void KSystemTray::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksystemtray.moc"
