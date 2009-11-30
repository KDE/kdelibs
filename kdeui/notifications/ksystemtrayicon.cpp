/* This file is part of the KDE libraries

    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)
    Copyright (c) 2007      by Charles Connell <charles@connells.org>
    Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

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

#include "ksystemtrayicon.h"
#include "kaboutdata.h"
#include "kaction.h"
#include "kcomponentdata.h"
#include "klocale.h"
#include "kmenu.h"
#include "kmessagebox.h"
#include "kshortcut.h"
#include "kactioncollection.h"
#include "kstandardaction.h"
#include <kwindowsystem.h>

#ifdef Q_WS_X11
#include <QX11Info>
#endif
#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include <kiconloader.h>
#include <kapplication.h>
#include <kconfiggroup.h>

#include <QMouseEvent>
#include <QToolButton>
#include <QMovie>
#include <QPointer>

#ifdef Q_WS_WIN
class KSystemTrayIconPrivate : public QObject
#else
class KSystemTrayIconPrivate
#endif
{
public:
    KSystemTrayIconPrivate(KSystemTrayIcon* trayIcon, QWidget* parent)
        : q(trayIcon)
    {
        actionCollection = new KActionCollection( trayIcon );
        hasQuit = false;
        onAllDesktops = false;
        window = parent;
        movie = 0;
#ifdef Q_WS_WIN
		if ( window ) {
            window->installEventFilter( this );
		}
#endif
    }

    ~KSystemTrayIconPrivate()
    {
#ifdef Q_WS_WIN
		if ( window ) {
            window->removeEventFilter( this );
		}
#endif
        delete actionCollection;
        delete menu;
    }


    void _k_slotNewFrame()
    {
        q->setIcon(QIcon(movie->currentPixmap()));
    }

#ifdef Q_WS_WIN
    bool eventFilter(QObject *obj, QEvent *ev)
    {
      if(ev->type() == QEvent::ActivationChange) {
        dwTickCount = GetTickCount();
      }
      return QObject::eventFilter(obj, ev);
    }
    DWORD dwTickCount;
#endif

    KSystemTrayIcon* q;
    KActionCollection* actionCollection;
    KMenu* menu;
    QWidget* window;
    QAction* titleAction;
    bool onAllDesktops : 1; // valid only when the parent widget was hidden
    bool hasQuit : 1;
    QPointer<QMovie> movie;
};

KSystemTrayIcon::KSystemTrayIcon( QWidget* parent )
    : QSystemTrayIcon( parent ),
      d( new KSystemTrayIconPrivate( this, parent ) )
{
    init( parent );
}

KSystemTrayIcon::KSystemTrayIcon( const QString& icon, QWidget* parent )
    : QSystemTrayIcon( loadIcon( icon ), parent ),
      d( new KSystemTrayIconPrivate( this, parent ) )
{
    init( parent );
}

KSystemTrayIcon::KSystemTrayIcon( const QIcon& icon, QWidget* parent )
    : QSystemTrayIcon( icon, parent ),
      d( new KSystemTrayIconPrivate( this, parent ) )
{
    init( parent );
}

KSystemTrayIcon::KSystemTrayIcon(QMovie* movie, QWidget *parent)
    : QSystemTrayIcon(parent),
      d( new KSystemTrayIconPrivate( this, parent ) )
{
    init(parent);
    setMovie(movie);
}

void KSystemTrayIcon::init( QWidget* parent )
{
    // Ensure that closing the last KMainWindow doesn't exit the application
    // if a system tray icon is still present.
    KGlobal::ref();
    d->menu = new KMenu( parent );
    d->titleAction = d->menu->addTitle( qApp->windowIcon(), KGlobal::caption() );
    d->menu->setTitle( KGlobal::mainComponent().aboutData()->programName() );
    connect( d->menu, SIGNAL( aboutToShow() ), this, SLOT( contextMenuAboutToShow() ) );
    setContextMenu( d->menu );

    KStandardAction::quit( this, SLOT( maybeQuit() ), d->actionCollection );

    if ( parent )
    {
        QAction *action = d->actionCollection->addAction("minimizeRestore");
        action->setText(i18n("Minimize"));
        connect( action, SIGNAL( triggered( bool ) ), this, SLOT( minimizeRestoreAction() ) );

#ifdef Q_WS_X11
        KWindowInfo info = KWindowSystem::windowInfo( parent->winId(), NET::WMDesktop );
        d->onAllDesktops = info.onAllDesktops();
#else
        d->onAllDesktops = false;
#endif
    }
    else
    {
        d->onAllDesktops = false;
    }

    connect( this, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
             SLOT( activateOrHide( QSystemTrayIcon::ActivationReason ) ) );
}

QWidget *KSystemTrayIcon::parentWidget() const
{
    return d->window;
}

KSystemTrayIcon::~KSystemTrayIcon()
{
    delete d;
    KGlobal::deref();
}

void KSystemTrayIcon::contextMenuAboutToShow( )
{
    if ( !d->hasQuit )
    {
        // we need to add the actions to the menu afterwards so that these items
        // appear at the _END_ of the menu
        d->menu->addSeparator();
        QAction* action = d->actionCollection->action( "minimizeRestore" );

        if ( action )
        {
            d->menu->addAction( action );
        }

        action = d->actionCollection->action( KStandardAction::name( KStandardAction::Quit ) );

        if ( action )
        {
            d->menu->addAction( action );
        }

        d->hasQuit = true;
    }

    if ( d->window )
    {
        QAction* action = d->actionCollection->action("minimizeRestore");
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
void KSystemTrayIcon::minimizeRestoreAction()
{
    if ( d->window )
    {
        bool restore = !( d->window->isVisible() );
        minimizeRestore( restore );
    }
}

void KSystemTrayIcon::maybeQuit()
{
    QString caption = KGlobal::caption();
    QString query = i18n("<qt>Are you sure you want to quit <b>%1</b>?</qt>",
                         caption);
    if (KMessageBox::warningContinueCancel(d->window, query,
                                     i18n("Confirm Quit From System Tray"),
                                     KStandardGuiItem::quit(),
                                     KStandardGuiItem::cancel(),
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
void KSystemTrayIcon::activateOrHide( QSystemTrayIcon::ActivationReason reasonCalled )
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
#ifdef Q_WS_WIN
    // the problem is that we lose focus when the systray icon is activated
    // and we don't know the former active window
    // therefore we watch for activation event and use our stopwatch :)
    if( GetTickCount() - d->dwTickCount < 300 ) {
        // we were active in the last 300ms -> hide it
        minimizeRestore( false );
    } else {
        minimizeRestore( true );
    }
#elif defined(Q_WS_X11)
    KWindowInfo info1 = KWindowSystem::windowInfo( pw->winId(), NET::XAWMState | NET::WMState );
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
        QListIterator< WId > it (KWindowSystem::stackingOrder());
        it.toBack();
        while( it.hasPrevious() )
        {
            WId id = it.previous();
            if( id == pw->winId() )
                break;
            KWindowInfo info2 = KWindowSystem::windowInfo( id,
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
            KWindowSystem::activateWindow( pw->winId());
            return;
        }
        minimizeRestore( false ); // hide
    }
#endif
}

void KSystemTrayIcon::minimizeRestore( bool restore )
{
    QWidget* pw = d->window;
    if (!pw)
        return;
#ifdef Q_WS_X11
    KWindowInfo info = KWindowSystem::windowInfo(pw->winId(), NET::WMGeometry | NET::WMDesktop);
    if (restore) {
        if (d->onAllDesktops) {
            KWindowSystem::setOnAllDesktops(pw->winId(), true);
        } else {
            KWindowSystem::setCurrentDesktop(info.desktop());
        }
        pw->move(info.geometry().topLeft()); // avoid placement policies
        pw->show();
        pw->raise();
        KWindowSystem::activateWindow(pw->winId());
    } else {
        d->onAllDesktops = info.onAllDesktops();
        pw->hide();
    }
#else
    if ( restore )
    {
        pw->show();
        pw->raise();
        KWindowSystem::forceActiveWindow( pw->winId() );
    } else {
        pw->hide();
    }
#endif
}

KActionCollection* KSystemTrayIcon::actionCollection()
{
    return d->actionCollection;
}

QIcon KSystemTrayIcon::loadIcon(const QString &icon, const KComponentData &componentData)
{
    KConfigGroup cg(componentData.config(), "System Tray");
    const int iconWidth = cg.readEntry("systrayIconWidth", 22);
    return KIconLoader::global()->loadIcon( icon, KIconLoader::Panel, iconWidth );
}

void KSystemTrayIcon::toggleActive()
{
    activateOrHide( QSystemTrayIcon::Trigger );
}

bool KSystemTrayIcon::parentWidgetTrayClose() const
{
    if( kapp != NULL && kapp->sessionSaving())
        return false; // normal close
    return true;
}

void KSystemTrayIcon::setContextMenuTitle(QAction *action)
{
    // can never be null, and is always the requsted type, so no need to do null checks after casts.
    QToolButton *button = static_cast<QToolButton*>((static_cast<QWidgetAction*>(d->titleAction))->defaultWidget());
    button->setDefaultAction(action);
}

QAction *KSystemTrayIcon::contextMenuTitle() const
{
    QToolButton *button = static_cast<QToolButton*>((static_cast<QWidgetAction*>(d->titleAction))->defaultWidget());
    return button->defaultAction();
}

void KSystemTrayIcon::setMovie(QMovie* m)
{
    if (d->movie == m) {
        return;
    }
    delete d->movie;
    m->setParent(this);
    d->movie = m;
    connect(d->movie, SIGNAL(frameChanged(int)), this, SLOT(_k_slotNewFrame()));
    d->movie->setCacheMode(QMovie::CacheAll);
}

const QMovie* KSystemTrayIcon::movie() const
{
    return d->movie;
}

#include "ksystemtrayicon.moc"
