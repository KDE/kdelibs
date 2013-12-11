/*
    This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 2007 Lubos Lunak (l.lunak@kde.org)

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

#include "kwindowsystem.h"

#include <QDebug>
#include <kxerrorhandler_p.h>
#include <kxutils_p.h>
#include <netwm.h>
#include <fixx11h.h>

#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QBitmap>
#include <QDesktopWidget>
#include <QIcon>
#include <QMetaMethod>
#include <QWindow>
#include <QX11Info>

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>
#include <xcb/xfixes.h>

#include <config-kwindowsystem.h>

#if HAVE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif

class KWindowSystemStaticContainer {
public:
    KWindowSystemStaticContainer() : d(0) {}
    KWindowSystem kwm;
    KWindowSystemPrivate* d;
};


Q_GLOBAL_STATIC(KWindowSystemStaticContainer, g_kwmInstanceContainer)

static Atom net_wm_cm;
static void create_atoms( Display* dpy = QX11Info::display() );

static unsigned long windows_properties[ 2 ] = { NET::ClientList | NET::ClientListStacking |
                                     NET::Supported |
				     NET::NumberOfDesktops |
				     NET::DesktopGeometry |
                                     NET::DesktopViewport |
				     NET::CurrentDesktop |
				     NET::DesktopNames |
				     NET::ActiveWindow |
				     NET::WorkArea, /**/
                                     NET::WM2ShowingDesktop };

// ClientList and ClientListStacking is not per-window information, but a desktop information,
// so track it even with only INFO_BASIC
static unsigned long desktop_properties[ 2 ] = { NET::ClientList | NET::ClientListStacking |
                                     NET::Supported |
				     NET::NumberOfDesktops |
				     NET::DesktopGeometry |
                                     NET::DesktopViewport |
				     NET::CurrentDesktop |
				     NET::DesktopNames |
				     NET::ActiveWindow |
				     NET::WorkArea, /**/
                                     NET::WM2ShowingDesktop };

class KWindowSystemPrivate
    : public QWidget, public NETRootInfo, public QAbstractNativeEventFilter
{
public:
    KWindowSystemPrivate(int _what);
    void activate();
    QList<WId> windows;
    QList<WId> stackingOrder;

    struct StrutData
    {
        StrutData( WId window_, const NETStrut& strut_, int desktop_ )
            : window( window_ ), strut( strut_ ), desktop( desktop_ ) {}
        StrutData() {} // for QValueList to be happy
        WId window;
        NETStrut strut;
        int desktop;
    };
    QList<StrutData> strutWindows;
    QList<WId> possibleStrutWindows;
    bool strutSignalConnected;
    bool compositingEnabled;
    bool haveXfixes;
    int what;
    int xfixesEventBase;
    bool mapViewport();

    virtual bool nativeEventFilter(const QByteArray& eventType, void* message, long int* result) Q_DECL_OVERRIDE;

    void updateStackingOrder();
    bool removeStrutWindow( WId );

protected:
    virtual void addClient(xcb_window_t) Q_DECL_OVERRIDE;
    virtual void removeClient(xcb_window_t) Q_DECL_OVERRIDE;

private:
    bool nativeEventFilter(xcb_generic_event_t *event);
};

KWindowSystemPrivate::KWindowSystemPrivate(int _what)
    : QWidget(0),
      NETRootInfo( QX11Info::connection(),
                   _what >= KWindowSystem::INFO_WINDOWS ? windows_properties : desktop_properties,
                   2, -1, false ),
      QAbstractNativeEventFilter(),
      strutSignalConnected( false ),
      haveXfixes( false ),
      what( _what )
{
    QCoreApplication::instance()->installNativeEventFilter(this);
    (void ) qApp->desktop(); //trigger desktop widget creation to select root window events

#if HAVE_XFIXES
    int errorBase;
    if ((haveXfixes = XFixesQueryExtension(QX11Info::display(), &xfixesEventBase, &errorBase))) {
        create_atoms();
        XFixesSelectSelectionInput(QX11Info::display(), winId(), net_wm_cm,
                                   XFixesSetSelectionOwnerNotifyMask |
                                   XFixesSelectionWindowDestroyNotifyMask |
                                   XFixesSelectionClientCloseNotifyMask);
        compositingEnabled = XGetSelectionOwner(QX11Info::display(), net_wm_cm) != None;
    }
#endif
}

// not virtual, but it's called directly only from init()
void KWindowSystemPrivate::activate()
{
    NETRootInfo::activate();
    updateStackingOrder();
}

bool KWindowSystemPrivate::nativeEventFilter(const QByteArray& eventType, void* message, long int* result)
{
    Q_UNUSED(result)
    if (eventType != "xcb_generic_event_t") {
        // only interested in XCB events of course
        return false;
    }
    return nativeEventFilter(reinterpret_cast<xcb_generic_event_t*>(message));
}

bool KWindowSystemPrivate::nativeEventFilter(xcb_generic_event_t *ev)
{
    KWindowSystem* s_q = KWindowSystem::self();
    const uint8_t eventType = ev->response_type & ~0x80;

#ifdef HAVE_XFIXES
    if (eventType == xfixesEventBase + XCB_XFIXES_SELECTION_NOTIFY) {
        xcb_xfixes_selection_notify_event_t *event = reinterpret_cast<xcb_xfixes_selection_notify_event_t*>(ev);
        if (event->window == winId()) {
            bool haveOwner = event->owner != XCB_WINDOW_NONE;
            if (compositingEnabled != haveOwner) {
                compositingEnabled = haveOwner;
                emit s_q->compositingChanged(compositingEnabled);
            }
            return true;
        }
        // Qt compresses XFixesSelectionNotifyEvents without caring about the actual window
        // gui/kernel/qapplication_x11.cpp
        // until that can be assumed fixed, we also react on events on the root (caused by Qts own compositing tracker)
        if (event->window == QX11Info::appRootWindow()) {
            if (event->selection == net_wm_cm) {
                bool haveOwner = event->owner != XCB_WINDOW_NONE;
                if (compositingEnabled != haveOwner) {
                    compositingEnabled = haveOwner;
                    emit s_q->compositingChanged( compositingEnabled );
                }
                // NOTICE this is not our event, we just randomly captured it from Qt -> pass on
                return false;
            }
        }
        return false;
    }
#endif

    xcb_window_t eventWindow = XCB_WINDOW_NONE;
    switch (eventType) {
    case XCB_CLIENT_MESSAGE:
        eventWindow = reinterpret_cast<xcb_client_message_event_t*>(ev)->window;
        break;
    case XCB_PROPERTY_NOTIFY:
        eventWindow = reinterpret_cast<xcb_property_notify_event_t*>(ev)->window;
        break;
    case XCB_CONFIGURE_NOTIFY:
        eventWindow = reinterpret_cast<xcb_configure_notify_event_t*>(ev)->window;
        break;
    }

    if (eventWindow == QX11Info::appRootWindow()) {
        int old_current_desktop = currentDesktop();
        xcb_window_t old_active_window = activeWindow();
        int old_number_of_desktops = numberOfDesktops();
        bool old_showing_desktop = showingDesktop();
        unsigned long m[ 5 ];
        NETRootInfo::event( ev, m, 5 );

        if ((m[ PROTOCOLS ] & CurrentDesktop ) && currentDesktop() != old_current_desktop) {
            emit s_q->currentDesktopChanged( currentDesktop() );
        }
        if ((m[ PROTOCOLS ] & DesktopViewport ) && mapViewport() && currentDesktop() != old_current_desktop) {
            emit s_q->currentDesktopChanged( currentDesktop() );
        }
        if ((m[ PROTOCOLS ] & ActiveWindow ) && activeWindow() != old_active_window) {
            emit s_q->activeWindowChanged( activeWindow() );
        }
        if (m[ PROTOCOLS ] & DesktopNames) {
            emit s_q->desktopNamesChanged();
        }
        if ((m[ PROTOCOLS ] & NumberOfDesktops ) && numberOfDesktops() != old_number_of_desktops) {
            emit s_q->numberOfDesktopsChanged( numberOfDesktops() );
        }
        if ((m[ PROTOCOLS ] & DesktopGeometry ) && mapViewport() && numberOfDesktops() != old_number_of_desktops) {
            emit s_q->numberOfDesktopsChanged( numberOfDesktops() );
        }
        if (m[ PROTOCOLS ] & WorkArea) {
            emit s_q->workAreaChanged();
        }
        if (m[ PROTOCOLS ] & ClientListStacking) {
            updateStackingOrder();
            emit s_q->stackingOrderChanged();
        }
        if ((m[ PROTOCOLS2 ] & WM2ShowingDesktop ) && showingDesktop() != old_showing_desktop) {
            emit s_q->showingDesktopChanged( showingDesktop());
        }
    } else if (windows.contains(eventWindow)) {
        NETWinInfo ni(QX11Info::connection(), eventWindow, QX11Info::appRootWindow(), 0);
        unsigned long dirty[ 2 ];
        ni.event( ev, dirty, 2 );
        if (eventType == XCB_PROPERTY_NOTIFY) {
            xcb_property_notify_event_t *event = reinterpret_cast<xcb_property_notify_event_t*>(ev);
            if (event->atom == XCB_ATOM_WM_HINTS) {
                dirty[ NETWinInfo::PROTOCOLS ] |= NET::WMIcon; // support for old icons
            } else if (event->atom == XCB_ATOM_WM_NAME) {
                dirty[ NETWinInfo::PROTOCOLS ] |= NET::WMName; // support for old name
            } else if (event->atom == XCB_ATOM_WM_ICON_NAME)
                dirty[ NETWinInfo::PROTOCOLS ] |= NET::WMIconName; // support for old iconic name
        }
        if (mapViewport() && ( dirty[ NETWinInfo::PROTOCOLS ] & (NET::WMState | NET::WMGeometry))) {
            /* geometry change -> possible viewport change
             * state change -> possible NET::Sticky change
             */
            dirty[ NETWinInfo::PROTOCOLS ] |= NET::WMDesktop;
        }
        if ((dirty[ NETWinInfo::PROTOCOLS ] & NET::WMStrut) != 0) {
            removeStrutWindow(eventWindow);
            if (!possibleStrutWindows.contains(eventWindow)) {
                possibleStrutWindows.append(eventWindow);
            }
        }
        if (dirty[ NETWinInfo::PROTOCOLS ] || dirty[ NETWinInfo::PROTOCOLS2 ]) {
            emit s_q->windowChanged(eventWindow);
            emit s_q->windowChanged(eventWindow, dirty );
            emit s_q->windowChanged(eventWindow, dirty[ NETWinInfo::PROTOCOLS ] );
            if ((dirty[ NETWinInfo::PROTOCOLS ] & NET::WMStrut) != 0) {
                emit s_q->strutChanged();
            }
        }
    }

    return false;
}

bool KWindowSystemPrivate::removeStrutWindow( WId w )
{
    for( QList< StrutData >::Iterator it = strutWindows.begin();
         it != strutWindows.end();
         ++it )
        if( (*it).window == w ) {
            strutWindows.erase( it );
            return true;
        }
    return false;
}

void KWindowSystemPrivate::updateStackingOrder()
{
    stackingOrder.clear();
    for ( int i = 0; i <  clientListStackingCount(); i++ )
	stackingOrder.append( clientListStacking()[i] );
}

void KWindowSystemPrivate::addClient(xcb_window_t w)
{
    KWindowSystem* s_q = KWindowSystem::self();

    if ((what >= KWindowSystem::INFO_WINDOWS)) {
        xcb_connection_t *c = QX11Info::connection();
        QScopedPointer<xcb_get_window_attributes_reply_t, QScopedPointerPodDeleter> attr(xcb_get_window_attributes_reply(c,
            xcb_get_window_attributes_unchecked(c, w), Q_NULLPTR));

        uint32_t events = XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
        if (!attr.isNull()) {
            events = events | attr->your_event_mask;
        }
        xcb_change_window_attributes(c, w, XCB_CW_EVENT_MASK, &events);
    }

    bool emit_strutChanged = false;

    if( strutSignalConnected ) {
        NETWinInfo info( QX11Info::connection(), w, QX11Info::appRootWindow(), NET::WMStrut | NET::WMDesktop );
        NETStrut strut = info.strut();
        if ( strut.left || strut.top || strut.right || strut.bottom ) {
            strutWindows.append( StrutData( w, strut, info.desktop()));
            emit_strutChanged = true;
        }
    } else
        possibleStrutWindows.append( w );

    windows.append( w );
    emit s_q->windowAdded( w );
    if ( emit_strutChanged )
        emit s_q->strutChanged();
}

void KWindowSystemPrivate::removeClient(xcb_window_t w)
{
    KWindowSystem* s_q = KWindowSystem::self();

    bool emit_strutChanged = removeStrutWindow( w );
    if( strutSignalConnected && possibleStrutWindows.contains( w )) {
        NETWinInfo info( QX11Info::connection(), w, QX11Info::appRootWindow(), NET::WMStrut );
        NETStrut strut = info.strut();
        if ( strut.left || strut.top || strut.right || strut.bottom ) {
            emit_strutChanged = true;
        }
    }

    possibleStrutWindows.removeAll( w );
    windows.removeAll( w );
    emit s_q->windowRemoved( w );
    if ( emit_strutChanged )
        emit s_q->strutChanged();
}

bool KWindowSystemPrivate::mapViewport()
{
// compiz claims support even though it doesn't use virtual desktops :(
//    if( isSupported( NET::DesktopViewport ) && !isSupported( NET::NumberOfDesktops ))

// this test is duplicated in KWindowSystem::mapViewport()
    if( isSupported( NET::DesktopViewport ) && numberOfDesktops( true ) <= 1
        && ( desktopGeometry().width > QApplication::desktop()->width()
            || desktopGeometry().height > QApplication::desktop()->height()))
        return true;
    return false;
}

static bool atoms_created = false;

static Atom kde_wm_change_state;
static Atom _wm_protocols;
static Atom kwm_utf8_string;

static void create_atoms( Display* dpy ) {
    if (!atoms_created){
	const int max = 20;
	Atom* atoms[max];
	const char* names[max];
	Atom atoms_return[max];
	int n = 0;

	atoms[n] = &kde_wm_change_state;
	names[n++] = "_KDE_WM_CHANGE_STATE";

        atoms[n] = &_wm_protocols;
        names[n++] = "WM_PROTOCOLS";

        atoms[n] = &kwm_utf8_string;
        names[n++] = "UTF8_STRING";

        char net_wm_cm_name[ 100 ];
        sprintf( net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen( dpy ));
        atoms[n] = &net_wm_cm;
        names[n++] = net_wm_cm_name;

	// we need a const_cast for the shitty X API
	XInternAtoms( dpy, const_cast<char**>(names), n, false, atoms_return );
	for (int i = 0; i < n; i++ )
	    *atoms[i] = atoms_return[i];

	atoms_created = True;
    }
}

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
  XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, mask, &ev);
}

KWindowSystem* KWindowSystem::self()
{
    return &(g_kwmInstanceContainer()->kwm);
}


KWindowSystemPrivate* KWindowSystem::s_d_func()
{
    return g_kwmInstanceContainer()->d;
}


// optimalization - create KWindowSystemPrivate only when needed and only for what is needed
void KWindowSystem::connectNotify(const QMetaMethod& signal)
{
    int what = INFO_BASIC;
    if (signal == QMetaMethod::fromSignal(&KWindowSystem::workAreaChanged))
        what = INFO_WINDOWS;
    else if (signal == QMetaMethod::fromSignal(&KWindowSystem::strutChanged))
        what = INFO_WINDOWS;
    else if (signal == QMetaMethod::fromSignal(static_cast<void (KWindowSystem::*)(WId, const unsigned long*)>(&KWindowSystem::windowChanged)))
        what = INFO_WINDOWS;
    else if (signal == QMetaMethod::fromSignal(static_cast<void (KWindowSystem::*)(WId, uint)>(&KWindowSystem::windowChanged)))
        what = INFO_WINDOWS;
    else if (signal == QMetaMethod::fromSignal(static_cast<void (KWindowSystem::*)(WId)>(&KWindowSystem::windowChanged)))
        what = INFO_WINDOWS;

    init( what );
    KWindowSystemPrivate* const s_d = s_d_func();
    if (!s_d->strutSignalConnected && signal == QMetaMethod::fromSignal(&KWindowSystem::strutChanged))
        s_d->strutSignalConnected = true;

    QObject::connectNotify( signal );
}

// WARNING
// you have to call s_d_func() again after calling this function if you want a valid pointer!
void KWindowSystem::init(int what)
{
    KWindowSystemPrivate* const s_d = s_d_func();

    if (what >= INFO_WINDOWS)
       what = INFO_WINDOWS;
    else
       what = INFO_BASIC;

    if ( !s_d )
    {
        g_kwmInstanceContainer()->d = new KWindowSystemPrivate(what); // invalidates s_d
        g_kwmInstanceContainer()->d->activate();
    }
    else if (s_d->what < what)
    {
        delete s_d;
        g_kwmInstanceContainer()->d = new KWindowSystemPrivate(what); // invalidates s_d
        g_kwmInstanceContainer()->d->activate();
    }
}

const QList<WId>& KWindowSystem::windows()
{
    init( INFO_BASIC );
    return s_d_func()->windows;
}

KWindowInfo KWindowSystem::windowInfo( WId win, unsigned long properties, unsigned long properties2 )
{
    return KWindowInfo( win, properties, properties2 );
}

bool KWindowSystem::hasWId(WId w)
{
    init( INFO_BASIC );
    return s_d_func()->windows.contains( w );
}

QList<WId> KWindowSystem::stackingOrder()
{
    init( INFO_BASIC );
    return s_d_func()->stackingOrder;
}

int KWindowSystem::currentDesktop()
{
    if (!QX11Info::connection())
      return 1;

    if( mapViewport()) {
        init( INFO_BASIC );
        KWindowSystemPrivate* const s_d = s_d_func();
        NETPoint p = s_d->desktopViewport( s_d->currentDesktop( true ));
        return viewportToDesktop( QPoint( p.x, p.y ));
    }

    KWindowSystemPrivate* const s_d = s_d_func();
    if( s_d )
        return s_d->currentDesktop( true );
    NETRootInfo info( QX11Info::connection(), NET::CurrentDesktop );
    return info.currentDesktop( true );
}

int KWindowSystem::numberOfDesktops()
{
    if (!QX11Info::connection())
      return 1;

    if( mapViewport()) {
        init( INFO_BASIC );
        KWindowSystemPrivate* const s_d = s_d_func();
        NETSize s = s_d->desktopGeometry();
        return s.width / qApp->desktop()->width() * s.height / qApp->desktop()->height();
    }

    KWindowSystemPrivate* const s_d = s_d_func();
    if( s_d )
        return s_d->numberOfDesktops( true );
    NETRootInfo info( QX11Info::connection(), NET::NumberOfDesktops );
    return info.numberOfDesktops( true );
}

void KWindowSystem::setCurrentDesktop( int desktop )
{
    if( mapViewport()) {
        init( INFO_BASIC );
        KWindowSystemPrivate* const s_d = s_d_func();
        NETRootInfo info( QX11Info::connection(), 0 );
        QPoint pos = desktopToViewport( desktop, true );
        NETPoint p;
        p.x = pos.x();
        p.y = pos.y();
        info.setDesktopViewport( s_d->currentDesktop( true ), p );
        return;
    }
    NETRootInfo info( QX11Info::connection(), 0 );
    info.setCurrentDesktop( desktop, true );
}

void KWindowSystem::setOnAllDesktops( WId win, bool b )
{
    if( mapViewport()) {
        if( b )
            setState( win, NET::Sticky );
        else
            clearState( win, NET::Sticky );
        return;
    }
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMDesktop );
    if ( b )
	info.setDesktop( NETWinInfo::OnAllDesktops, true );
    else if ( info.desktop( true )  == NETWinInfo::OnAllDesktops ) {
	NETRootInfo rinfo( QX11Info::connection(), NET::CurrentDesktop );
	info.setDesktop( rinfo.currentDesktop( true ), true );
    }
}

void KWindowSystem::setOnDesktop( WId win, int desktop )
{
    if( mapViewport()) {
        if( desktop == NET::OnAllDesktops )
            return setOnAllDesktops( win, true );
        else
            clearState( win, NET::Sticky );
        init( INFO_BASIC );
        QPoint p = desktopToViewport( desktop, false );
        Window dummy;
        int x, y;
        unsigned int w, h, b, dp;
        XGetGeometry( QX11Info::display(), win, &dummy, &x, &y, &w, &h, &b, &dp );
        // get global position
        XTranslateCoordinates( QX11Info::display(), win, QX11Info::appRootWindow(), 0, 0, &x, &y, &dummy );
        x += w / 2; // center
        y += h / 2;
        // transform to coordinates on the current "desktop"
        x = x % qApp->desktop()->width();
        y = y % qApp->desktop()->height();
        if( x < 0 )
            x = x + qApp->desktop()->width();
        if( y < 0 )
            y = y + qApp->desktop()->height();
        x += p.x(); // move to given "desktop"
        y += p.y();
        x -= w / 2; // from center back to topleft
        y -= h / 2;
        p = constrainViewportRelativePosition( QPoint( x, y ));
        int flags = ( NET::FromTool << 12 ) | ( 0x03 << 8 ) | 10; // from tool(?), x/y, static gravity
        KWindowSystemPrivate* const s_d = s_d_func();
        s_d->moveResizeWindowRequest( win, flags, p.x(), p.y(), w, h );
        return;
    }
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMDesktop );
    info.setDesktop( desktop, true );
}

WId KWindowSystem::activeWindow()
{
    KWindowSystemPrivate* const s_d = s_d_func();
    if( s_d )
        return s_d->activeWindow();
    NETRootInfo info( QX11Info::connection(), NET::ActiveWindow );
    return info.activeWindow();
}

void KWindowSystem::activateWindow( WId win, long time )
{
    NETRootInfo info( QX11Info::connection(), 0 );
    if( time == 0 )
        time = QX11Info::appUserTime();
    info.setActiveWindow( win, NET::FromApplication, time,
        qApp->activeWindow() ? qApp->activeWindow()->winId() : 0 );
}

void KWindowSystem::forceActiveWindow( WId win, long time )
{
    NETRootInfo info( QX11Info::connection(), 0 );
    if( time == 0 )
        time = QX11Info::appTime();
    info.setActiveWindow( win, NET::FromTool, time, 0 );
}

void KWindowSystem::demandAttention( WId win, bool set )
{
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMState );
    info.setState( set ? NET::DemandsAttention : 0, NET::DemandsAttention );
}

WId KWindowSystem::transientFor( WId win )
{
    KXErrorHandler handler; // ignore badwindow
    Window transient_for = None;
    if( XGetTransientForHint( QX11Info::display(), win, &transient_for ))
        return transient_for;
    // XGetTransientForHint() did sync
    return None;
}

void KWindowSystem::setMainWindow( QWidget* subWidget, WId mainWindowId )
{
    // Set the WA_NativeWindow attribute to force the creation of the QWindow.
    // Without this QWidget::windowHandle() returns 0.
    subWidget->setAttribute(Qt::WA_NativeWindow, true);
    QWindow *subWindow = subWidget->windowHandle();
    Q_ASSERT(subWindow);

    QWindow *mainWindow = QWindow::fromWinId(mainWindowId);
    // mainWindow is not the child of any object, so make sure it gets deleted at some point
    connect(subWidget, &QObject::destroyed, mainWindow, &QObject::deleteLater);
    subWindow->setTransientParent(mainWindow);
}

WId KWindowSystem::groupLeader( WId win )
{
    KXErrorHandler handler; // ignore badwindow
    XWMHints *hints = XGetWMHints( QX11Info::display(), win );
    Window window_group = None;
    if ( hints )
    {
        if( hints->flags & WindowGroupHint )
            window_group = hints->window_group;
        XFree( reinterpret_cast< char* >( hints ));
    }
    // XGetWMHints() did sync
    return window_group;
}

QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale )
{
    return icon( win, width, height, scale, NETWM | WMHints | ClassHint | XApp );
}


QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale, int flags )
{
    KXErrorHandler handler; // ignore badwindow
    QPixmap result;
    if( flags & NETWM ) {
        NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMIcon );
        NETIcon ni = info.icon( width, height );
        if ( ni.data && ni.size.width > 0 && ni.size.height > 0 ) {
    	    QImage img( (uchar*) ni.data, (int) ni.size.width, (int) ni.size.height, QImage::Format_ARGB32 );
	    if ( scale && width > 0 && height > 0 &&img.size() != QSize( width, height ) && !img.isNull() )
	        img = img.scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
	    if ( !img.isNull() )
	        result = QPixmap::fromImage( img );
	    return result;
        }
    }

    if( flags & WMHints ) {
        Pixmap p = None;
        Pixmap p_mask = None;

        XWMHints *hints = XGetWMHints(QX11Info::display(), win );
        if (hints && (hints->flags & IconPixmapHint)){
    	    p = hints->icon_pixmap;
        }
        if (hints && (hints->flags & IconMaskHint)){
	    p_mask = hints->icon_mask;
        }
        if (hints)
	    XFree((char*)hints);

        if (p != None){
            QPixmap pm = KXUtils::createPixmapFromHandle( p, p_mask );
            if ( scale && width > 0 && height > 0 && !pm.isNull()
                 && ( pm.width() != width || pm.height() != height) ){
                result = QPixmap::fromImage( pm.toImage().scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
	    } else {
                result = pm;
	    }
        }
    }

    // Since width can be any arbitrary size, but the icons cannot,
    // take the nearest value for best results (ignoring 22 pixel
    // icons as they don't exist for apps):
    int iconWidth;
    if( width < 24 )
        iconWidth = 16;
    else if( width < 40 )
        iconWidth = 32;
    else
        iconWidth = 48;

    if( flags & ClassHint ) {
        // Try to load the icon from the classhint if the app didn't specify
        // its own:
        if( result.isNull() ) {

	    XClassHint	hint;
	    if( XGetClassHint( QX11Info::display(), win, &hint ) ) {
	        QString className = QLatin1String(hint.res_class);

                const QIcon icon = QIcon::fromTheme(className.toLower());
                const QPixmap pm = icon.isNull() ? QPixmap() : icon.pixmap(iconWidth, iconWidth);
	        if( scale && !pm.isNull() )
		    result = QPixmap::fromImage( pm.toImage().scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
	        else
		    result = pm;

	        XFree( hint.res_name );
	        XFree( hint.res_class );
	    }
        }
    }

    if( flags & XApp ) {
        // If the icon is still a null pixmap, load the icon for X applications
        // as a last resort:
        if ( result.isNull() ) {
            const QIcon icon = QIcon::fromTheme(QLatin1String("xorg"));
            const QPixmap pm = icon.isNull() ? QPixmap() : icon.pixmap(iconWidth, iconWidth);
	    if( scale && !pm.isNull() )
		result = QPixmap::fromImage( pm.toImage().scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
	    else
		result = pm;
        }
    }
    return result;
}

void KWindowSystem::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon )
{
    if ( icon.isNull() )
	return;
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), 0 );
    QImage img = icon.toImage().convertToFormat( QImage::Format_ARGB32 );
    NETIcon ni;
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, true );
    if ( miniIcon.isNull() )
	return;
    img = miniIcon.toImage().convertToFormat( QImage::Format_ARGB32 );
    if ( img.isNull() )
        return;
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, false );
}

void KWindowSystem::setType( WId win, NET::WindowType windowType )
{
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), 0 );
    info.setWindowType( windowType );
}

void KWindowSystem::setState( WId win, unsigned long state )
{
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMState );
    info.setState( state, state );
}

void KWindowSystem::clearState( WId win, unsigned long state )
{
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), NET::WMState );
    info.setState( 0, state );
}

void KWindowSystem::minimizeWindow( WId win, bool animation)
{
    if ( !animation )
    {
        create_atoms();
	sendClientMessageToRoot( win, kde_wm_change_state, IconicState, 1 );
    }
    XIconifyWindow( QX11Info::display(), win, QX11Info::appScreen() );
}

void KWindowSystem::unminimizeWindow( WId win, bool animation )
{
    if ( !animation )
    {
        create_atoms();
	sendClientMessageToRoot( win, kde_wm_change_state, NormalState, 1 );
    }
    XMapWindow( QX11Info::display(), win );
}

void KWindowSystem::raiseWindow( WId win )
{
    NETRootInfo info( QX11Info::connection(), NET::Supported );
    if( info.isSupported( NET::WM2RestackWindow ))
        info.restackRequest( win, NET::FromTool, None, Above, QX11Info::appUserTime());
    else
        XRaiseWindow( QX11Info::display(), win );
}

void KWindowSystem::lowerWindow( WId win )
{
    NETRootInfo info( QX11Info::connection(), NET::Supported );
    if( info.isSupported( NET::WM2RestackWindow ))
        info.restackRequest( win, NET::FromTool, None, Below, QX11Info::appUserTime());
    else
        XLowerWindow( QX11Info::display(), win );
}

bool KWindowSystem::compositingActive()
{
    if( QX11Info::display()) {
        init( INFO_BASIC );
        if (s_d_func()->haveXfixes) {
            return s_d_func()->compositingEnabled;
        } else {
            create_atoms();
            return XGetSelectionOwner( QX11Info::display(), net_wm_cm );
        }
    } else { // work even without QApplication instance
        Display* dpy = XOpenDisplay( NULL );
        create_atoms( dpy );
        bool ret = XGetSelectionOwner( dpy, net_wm_cm ) != None;
        XCloseDisplay( dpy );
        return ret;
    }
}

QRect KWindowSystem::workArea( int desktop )
{
    init( INFO_BASIC );
    int desk  = (desktop > 0 && desktop <= (int) s_d_func()->numberOfDesktops() ) ? desktop : currentDesktop();
    if ( desk <= 0 )
        return QApplication::desktop()->geometry();

    NETRect r = s_d_func()->workArea( desk );
    if( r.size.width <= 0 || r.size.height <= 0 ) // not set
        return QApplication::desktop()->geometry();

    return QRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
}

QRect KWindowSystem::workArea( const QList<WId>& exclude, int desktop )
{
    init( INFO_WINDOWS ); // invalidates s_d_func's return value
    KWindowSystemPrivate* const s_d = s_d_func();

    QRect all = QApplication::desktop()->geometry();
    QRect a = all;

    if (desktop == -1)
        desktop = s_d->currentDesktop();

    QList<WId>::ConstIterator it1;
    for( it1 = s_d->windows.constBegin(); it1 != s_d->windows.constEnd(); ++it1 ) {

        if(exclude.contains(*it1))
            continue;

// Kicker (very) extensively calls this function, causing hundreds of roundtrips just
// to repeatedly find out struts of all windows. Therefore strut values for strut
// windows are cached here.
        NETStrut strut;
        QList< KWindowSystemPrivate::StrutData >::Iterator it2 = s_d->strutWindows.begin();
        for( ; it2 != s_d->strutWindows.end(); ++it2 )
            if( (*it2).window == *it1 )
                break;

            if( it2 != s_d->strutWindows.end()) {
                if(!((*it2).desktop == desktop || (*it2).desktop == NETWinInfo::OnAllDesktops ))
                    continue;

                strut = (*it2).strut;
            } else if( s_d->possibleStrutWindows.contains( *it1 ) ) {

                NETWinInfo info( QX11Info::connection(), (*it1), QX11Info::appRootWindow(), NET::WMStrut | NET::WMDesktop);
                strut = info.strut();
                s_d->possibleStrutWindows.removeAll( *it1 );
                s_d->strutWindows.append( KWindowSystemPrivate::StrutData( *it1, info.strut(), info.desktop()));

                if( !(info.desktop() == desktop || info.desktop() == NETWinInfo::OnAllDesktops) )
                    continue;
            } else
                continue; // not a strut window

        QRect r = all;
        if ( strut.left > 0 )
            r.setLeft( r.left() + (int) strut.left );
        if ( strut.top > 0 )
            r.setTop( r.top() + (int) strut.top );
        if ( strut.right > 0  )
            r.setRight( r.right() - (int) strut.right );
        if ( strut.bottom > 0  )
            r.setBottom( r.bottom() - (int) strut.bottom );

        a = a.intersected(r);
    }
    return a;
}

QString KWindowSystem::desktopName( int desktop )
{
    init( INFO_BASIC );
    KWindowSystemPrivate* const s_d = s_d_func();

    bool isDesktopSane = (desktop > 0 && desktop <= (int) s_d->numberOfDesktops());
    const char* name = s_d->desktopName( isDesktopSane ? desktop : currentDesktop() );

    if ( name && name[0] )
        return QString::fromUtf8( name );

    return tr("Desktop %1").arg(desktop);
}

void KWindowSystem::setDesktopName( int desktop, const QString& name )
{
    KWindowSystemPrivate* const s_d = s_d_func();

    if (desktop <= 0 || desktop > (int) numberOfDesktops() )
        desktop = currentDesktop();

    if( s_d ) {
        s_d->setDesktopName( desktop, name.toUtf8().constData() );
        return;
    }

    NETRootInfo info( QX11Info::connection(), 0 );
    info.setDesktopName( desktop, name.toUtf8().constData() );
}

bool KWindowSystem::showingDesktop()
{
    init( INFO_BASIC );
    return s_d_func()->showingDesktop();
}

void KWindowSystem::setUserTime( WId win, long time )
{
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), 0 );
    info.setUserTime( time );
}

void KWindowSystem::setExtendedStrut( WId win, int left_width, int left_start, int left_end,
    int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
    int bottom_width, int bottom_start, int bottom_end )
{
    NETWinInfo info( QX11Info::connection(), win, QX11Info::appRootWindow(), 0 );
    NETExtendedStrut strut;
    strut.left_width = left_width;
    strut.right_width = right_width;
    strut.top_width = top_width;
    strut.bottom_width = bottom_width;
    strut.left_start = left_start;
    strut.left_end = left_end;
    strut.right_start = right_start;
    strut.right_end = right_end;
    strut.top_start = top_start;
    strut.top_end = top_end;
    strut.bottom_start = bottom_start;
    strut.bottom_end = bottom_end;
    info.setExtendedStrut( strut );
    NETStrut oldstrut;
    oldstrut.left = left_width;
    oldstrut.right = right_width;
    oldstrut.top = top_width;
    oldstrut.bottom = bottom_width;
    info.setStrut( oldstrut );
}

void KWindowSystem::setStrut( WId win, int left, int right, int top, int bottom )
{
    int w = XDisplayWidth( QX11Info::display(), DefaultScreen( QX11Info::display()));
    int h = XDisplayHeight( QX11Info::display(), DefaultScreen( QX11Info::display()));
    setExtendedStrut( win, left, 0, left != 0 ? w : 0, right, 0, right != 0 ? w : 0,
        top, 0, top != 0 ? h : 0, bottom, 0, bottom != 0 ? h : 0 );
}

bool KWindowSystem::icccmCompliantMappingState()
{
    static enum { noidea, yes, no } wm_is_1_2_compliant = noidea;
    if( wm_is_1_2_compliant == noidea ) {
        NETRootInfo info( QX11Info::connection(), NET::Supported );
        wm_is_1_2_compliant = info.isSupported( NET::Hidden ) ? yes : no;
    }
    return wm_is_1_2_compliant == yes;
}

bool KWindowSystem::allowedActionsSupported()
{
    static enum { noidea, yes, no } wm_supports_allowed_actions = noidea;
    if( wm_supports_allowed_actions == noidea ) {
        NETRootInfo info( QX11Info::connection(), NET::Supported );
        wm_supports_allowed_actions = info.isSupported( NET::WM2AllowedActions ) ? yes : no;
    }
    return wm_supports_allowed_actions == yes;
}

QString KWindowSystem::readNameProperty( WId win, unsigned long atom )
{
    XTextProperty tp;
    char **text = NULL;
    int count;
    QString result;
    if ( XGetTextProperty( QX11Info::display(), win, &tp, atom ) != 0 && tp.value != NULL ) {
        create_atoms();

        if ( tp.encoding == kwm_utf8_string ) {
            result = QString::fromUtf8 ( (const char*) tp.value );
        } else if ( XmbTextPropertyToTextList( QX11Info::display(), &tp, &text, &count) == Success &&
                  text != NULL && count > 0 ) {
            result = QString::fromLocal8Bit( text[0] );
        } else if ( tp.encoding == XA_STRING )
            result = QString::fromLocal8Bit( (const char*) tp.value );
        if( text != NULL )
            XFreeStringList( text );
        XFree( tp.value );
    }
    return result;
}

void KWindowSystem::allowExternalProcessWindowActivation( int pid )
{
    // Normally supported by X11, but may depend on some window managers ?
    Q_UNUSED(pid)
}

void KWindowSystem::setBlockingCompositing( WId window, bool active )
{
    NETWinInfo info( QX11Info::connection(), window, QX11Info::appRootWindow(), 0 );
    info.setBlockingCompositing( active );
}


bool KWindowSystem::mapViewport()
{
    KWindowSystemPrivate* const s_d = s_d_func();
    if( s_d )
        return s_d->mapViewport();
    // avoid creating KWindowSystemPrivate
    NETRootInfo infos( QX11Info::connection(), NET::Supported );
    if( !infos.isSupported( NET::DesktopViewport ))
        return false;
    NETRootInfo info( QX11Info::connection(), NET::NumberOfDesktops | NET::CurrentDesktop | NET::DesktopGeometry );
    if( info.numberOfDesktops( true ) <= 1
        && ( info.desktopGeometry().width > QApplication::desktop()->width()
            || info.desktopGeometry().height > QApplication::desktop()->height()))
        return true;
    return false;
}

int KWindowSystem::viewportToDesktop( const QPoint& p )
{
    init( INFO_BASIC );
    KWindowSystemPrivate* const s_d = s_d_func();
    NETSize s = s_d->desktopGeometry();
    QSize vs = qApp->desktop()->size();
    int xs = s.width / vs.width();
    int x = p.x() < 0 ? 0 : p.x() >= s.width ? xs - 1 : p.x() / vs.width();
    int ys = s.height / vs.height();
    int y = p.y() < 0 ? 0 : p.y() >= s.height ? ys - 1 : p.y() / vs.height();
    return y * xs + x + 1;
}

int KWindowSystem::viewportWindowToDesktop( const QRect& r )
{
    init( INFO_BASIC );
    KWindowSystemPrivate* const s_d = s_d_func();
    QPoint p = r.center();
    // make absolute
    p = QPoint( p.x() + s_d->desktopViewport( s_d->currentDesktop( true )).x,
        p.y() + s_d->desktopViewport( s_d->currentDesktop( true )).y );
    NETSize s = s_d->desktopGeometry();
    QSize vs = qApp->desktop()->size();
    int xs = s.width / vs.width();
    int x = p.x() < 0 ? 0 : p.x() >= s.width ? xs - 1 : p.x() / vs.width();
    int ys = s.height / vs.height();
    int y = p.y() < 0 ? 0 : p.y() >= s.height ? ys - 1 : p.y() / vs.height();
    return y * xs + x + 1;
}

QPoint KWindowSystem::desktopToViewport( int desktop, bool absolute )
{
    init( INFO_BASIC );
    KWindowSystemPrivate* const s_d = s_d_func();
    NETSize s = s_d->desktopGeometry();
    QSize vs = qApp->desktop()->size();
    int xs = s.width / vs.width();
    int ys = s.height / vs.height();
    if( desktop <= 0 || desktop > xs * ys )
        return QPoint( 0, 0 );
    --desktop;
    QPoint ret( vs.width() * ( desktop % xs ), vs.height() * ( desktop / xs ));
    if( !absolute ) {
        ret = QPoint( ret.x() - s_d->desktopViewport( s_d->currentDesktop( true )).x,
            ret.y() - s_d->desktopViewport( s_d->currentDesktop( true )).y );
        if( ret.x() >= s.width )
            ret.setX( ret.x() - s.width );
        if( ret.x() < 0 )
            ret.setX( ret.x() + s.width );
        if( ret.y() >= s.height )
            ret.setY( ret.y() - s.height );
        if( ret.y() < 0 )
            ret.setY( ret.y() + s.height );
    }
    return ret;
}

QPoint KWindowSystem::constrainViewportRelativePosition( const QPoint& pos )
{
    init( INFO_BASIC );
    KWindowSystemPrivate* const s_d = s_d_func();
    NETSize s = s_d->desktopGeometry();
    NETPoint c = s_d->desktopViewport( s_d->currentDesktop( true ));
    int x = ( pos.x() + c.x ) % s.width;
    int y = ( pos.y() + c.y ) % s.height;
    if( x < 0 )
        x += s.width;
    if( y < 0 )
        y += s.height;
    return QPoint( x - c.x, y - c.y );
}

#include "moc_kwindowsystem.cpp"
