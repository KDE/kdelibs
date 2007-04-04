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

#include "kwm.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kuniqueapplication.h>
#include <kxerrorhandler.h>
#include <netwm.h>
#include <qbitmap.h>
#include <QDesktopWidget>
#include <qdialog.h>
#include <QtDBus/QtDBus>
#include <qx11info_x11.h>
#include <X11/Xatom.h>

class KWMStaticContainer {
public:
    KWMStaticContainer() : d(0) {}
    KWM kwm;
    KWMPrivate* d;
};


K_GLOBAL_STATIC(KWMStaticContainer, g_kwmInstanceContainer)


static unsigned long windows_properties[ 2 ] = { NET::ClientList | NET::ClientListStacking |
				     NET::NumberOfDesktops |
				     NET::DesktopGeometry |
                                     NET::DesktopViewport |
				     NET::CurrentDesktop |
				     NET::DesktopNames |
				     NET::ActiveWindow |
				     NET::WorkArea |
				     NET::KDESystemTrayWindows,
                                     NET::WM2ShowingDesktop };

static unsigned long desktop_properties[ 2 ] = { NET::ClientList |
				     NET::NumberOfDesktops |
				     NET::DesktopGeometry |
                                     NET::DesktopViewport |
				     NET::CurrentDesktop |
				     NET::DesktopNames |
				     NET::ActiveWindow |
				     NET::WorkArea |
				     NET::KDESystemTrayWindows,
                                     NET::WM2ShowingDesktop };

class KWMPrivate
    : public QWidget, public NETRootInfo
{
public:
    KWMPrivate(int _what);
    QList<WId> windows;
    QList<WId> stackingOrder;
    QList<WId> systemTrayWindows;

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
    int what;

    void addClient(Window);
    void removeClient(Window);

    bool x11Event( XEvent * ev );

    void updateStackingOrder();
    bool removeStrutWindow( WId );

    int numberOfViewports(int desktop) const;
    int currentViewport(int desktop) const;
};

KWMPrivate::KWMPrivate(int _what)
    : QWidget(0),
      NETRootInfo( QX11Info::display(),
                   _what >= KWM::INFO_WINDOWS ? windows_properties : desktop_properties,
                   2, -1, false ),
      strutSignalConnected( false ),
      what( _what )
{
    kapp->installX11EventFilter( this );
    (void ) qApp->desktop(); //trigger desktop widget creation to select root window events
    activate();
    updateStackingOrder();
}

bool KWMPrivate::x11Event( XEvent * ev )
{
    KWM* s_q = KWM::self();

    if ( ev->xany.window == QX11Info::appRootWindow() ) {
        int old_current_desktop = currentDesktop();
        WId old_active_window = activeWindow();
        int old_number_of_desktops = numberOfDesktops();
        bool old_showing_desktop = showingDesktop();
        unsigned long m[ 5 ];
	NETRootInfo::event( ev, m, 5 );

	if (( m[ PROTOCOLS ] & CurrentDesktop ) && currentDesktop() != old_current_desktop )
	    emit s_q->currentDesktopChanged( currentDesktop() );
	if (( m[ PROTOCOLS ] & ActiveWindow ) && activeWindow() != old_active_window )
	    emit s_q->activeWindowChanged( activeWindow() );
 	if ( m[ PROTOCOLS ] & DesktopViewport )
 	    emit s_q->currentDesktopViewportChanged(currentDesktop(), currentViewport(currentDesktop()));
	if ( m[ PROTOCOLS ] & DesktopNames )
	    emit s_q->desktopNamesChanged();
	if (( m[ PROTOCOLS ] & NumberOfDesktops ) && numberOfDesktops() != old_number_of_desktops )
	    emit s_q->numberOfDesktopsChanged( numberOfDesktops() );
	if ( m[ PROTOCOLS ] & WorkArea )
	    emit s_q->workAreaChanged();
	if ( m[ PROTOCOLS ] & ClientListStacking ) {
	    updateStackingOrder();
	    emit s_q->stackingOrderChanged();
	}
        if(( m[ PROTOCOLS2 ] & WM2ShowingDesktop ) && showingDesktop() != old_showing_desktop ) {
	    emit s_q->showingDesktopChanged( showingDesktop());
        }
    } else  if ( windows.contains( ev->xany.window ) ){
	NETWinInfo ni( QX11Info::display(), ev->xany.window, QX11Info::appRootWindow(), 0 );
        unsigned long dirty[ 2 ];
	ni.event( ev, dirty, 2 );
	if ( ev->type ==PropertyNotify ) {
            if( ev->xproperty.atom == XA_WM_HINTS )
	        dirty[ NETWinInfo::PROTOCOLS ] |= NET::WMIcon; // support for old icons
            else if( ev->xproperty.atom == XA_WM_NAME )
                dirty[ NETWinInfo::PROTOCOLS ] |= NET::WMName; // support for old name
            else if( ev->xproperty.atom == XA_WM_ICON_NAME )
                dirty[ NETWinInfo::PROTOCOLS ] |= NET::WMIconName; // support for old iconic name
        }
	if ( (dirty[ NETWinInfo::PROTOCOLS ] & NET::WMStrut) != 0 ) {
            removeStrutWindow( ev->xany.window );
            if ( !possibleStrutWindows.contains( ev->xany.window ) )
        	possibleStrutWindows.append( ev->xany.window );
	}
	if ( dirty[ NETWinInfo::PROTOCOLS ] || dirty[ NETWinInfo::PROTOCOLS2 ] ) {
	    emit s_q->windowChanged( ev->xany.window );
	    emit s_q->windowChanged( ev->xany.window, dirty );
	    emit s_q->windowChanged( ev->xany.window, dirty[ NETWinInfo::PROTOCOLS ] );
	    if ( (dirty[ NETWinInfo::PROTOCOLS ] & NET::WMStrut) != 0 )
		emit s_q->strutChanged();
	}
    }

    return false;
}

bool KWMPrivate::removeStrutWindow( WId w )
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

void KWMPrivate::updateStackingOrder()
{
    stackingOrder.clear();
    for ( int i = 0; i <  clientListStackingCount(); i++ )
	stackingOrder.append( clientListStacking()[i] );
}

void KWMPrivate::addClient(Window w)
{
    KWM* s_q = KWM::self();

    if ( (what >= KWM::INFO_WINDOWS) && !QWidget::find( w ) )
        XSelectInput( QX11Info::display(), w, PropertyChangeMask | StructureNotifyMask );

    bool emit_strutChanged = false;

    if( strutSignalConnected ) {
        NETWinInfo info( QX11Info::display(), w, QX11Info::appRootWindow(), NET::WMStrut | NET::WMDesktop );
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

void KWMPrivate::removeClient(Window w)
{
    KWM* s_q = KWM::self();

    bool emit_strutChanged = removeStrutWindow( w );
    if( strutSignalConnected && possibleStrutWindows.contains( w )) {
        NETWinInfo info( QX11Info::display(), w, QX11Info::appRootWindow(), NET::WMStrut );
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

int KWMPrivate::numberOfViewports(int desktop) const
{
    NETSize netdesktop = desktopGeometry(desktop);

    return netdesktop.width / QApplication::desktop()->screenGeometry().width();
}

int KWMPrivate::currentViewport(int desktop) const
{
    NETPoint netviewport = desktopViewport(desktop);

    return 1+(netviewport.x / QApplication::desktop()->screenGeometry().width());
}


static bool atoms_created = false;

static Atom kde_wm_change_state;
static Atom wm_protocols;
static Atom kwm_utf8_string;
static Atom net_wm_cm;

static void create_atoms() {
    if (!atoms_created){
	const int max = 20;
	Atom* atoms[max];
	const char* names[max];
	Atom atoms_return[max];
	int n = 0;

	atoms[n] = &kde_wm_change_state;
	names[n++] = "_KDE_WM_CHANGE_STATE";

        atoms[n] = &wm_protocols;
        names[n++] = "WM_PROTOCOLS";
        
        atoms[n] = &kwm_utf8_string;
        names[n++] = "UTF8_STRING";

        char net_wm_cm_name[ 100 ];
        sprintf( net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen( QX11Info::display()));
        atoms[n] = &net_wm_cm;
        names[n++] = net_wm_cm_name;

	// we need a const_cast for the shitty X API
	XInternAtoms( QX11Info::display(), const_cast<char**>(names), n, false, atoms_return );
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

KWM* KWM::self()
{
    return &(g_kwmInstanceContainer->kwm);
}


KWMPrivate* const KWM::s_d_func()
{
    return g_kwmInstanceContainer->d;
}


// optimalization - create KWMPrivate only when needed and only for what is needed
void KWM::connectNotify( const char* signal )
{
    int what = INFO_BASIC;
    if( QLatin1String( signal ) == SIGNAL(workAreaChanged()))
        what = INFO_WINDOWS;
    else if( QLatin1String( signal ) == SIGNAL(strutChanged()))
        what = INFO_WINDOWS;
    else if( QLatin1String( signal ) == SIGNAL(stackingOrderChanged()))
        what = INFO_WINDOWS;
    else if( QLatin1String( signal ) == QMetaObject::normalizedSignature(SIGNAL(windowChanged(WId,const unsigned long*))).data())
        what = INFO_WINDOWS;
    else if( QLatin1String( signal ) ==  QMetaObject::normalizedSignature(SIGNAL(windowChanged(WId,unsigned int))).data())
        what = INFO_WINDOWS;
    else if( QLatin1String( signal ) ==  QMetaObject::normalizedSignature(SIGNAL(windowChanged(WId))).data())
        what = INFO_WINDOWS;

    init( what );
    KWMPrivate* const s_d = s_d_func();
    if( !s_d->strutSignalConnected && qstrcmp( signal, SIGNAL(strutChanged())) == 0 )
        s_d->strutSignalConnected = true;

    QObject::connectNotify( signal );
}

// WARNING
// you have to call s_d_func() again after calling this function if you want a valid pointer!
void KWM::init(int what)
{
    KWMPrivate* const s_d = s_d_func();

    if (what >= INFO_WINDOWS)
       what = INFO_WINDOWS;
    else
       what = INFO_BASIC;

    if ( !s_d )
        g_kwmInstanceContainer->d = new KWMPrivate(what); // invalidates s_d
    else if (s_d->what < what)
    {
        delete s_d;
        g_kwmInstanceContainer->d = new KWMPrivate(what); // invalidates s_d
    }
}

const QList<WId>& KWM::windows()
{
    init( INFO_BASIC );
    return s_d_func()->windows;
}

KWM::WindowInfo KWM::windowInfo( WId win, unsigned long properties, unsigned long properties2 )
{
    return WindowInfo( win, properties, properties2 );
}

bool KWM::hasWId(WId w)
{
    init( INFO_BASIC );
    return s_d_func()->windows.contains( w );
}

const QList<WId>& KWM::stackingOrder()
{
    init( INFO_BASIC );
    return s_d_func()->stackingOrder;
}

int KWM::currentDesktop()
{
    if (!QX11Info::display())
      return 1;

    KWMPrivate* const s_d = s_d_func();
    if( s_d )
        return s_d->currentDesktop();
    NETRootInfo info( QX11Info::display(), NET::CurrentDesktop );
    return info.currentDesktop();
}

int KWM::numberOfDesktops()
{
    if (!QX11Info::display())
      return 1;

    KWMPrivate* const s_d = s_d_func();
    if( s_d )
        return s_d->numberOfDesktops();
    NETRootInfo info( QX11Info::display(), NET::NumberOfDesktops );
    return info.numberOfDesktops();
}

void KWM::setCurrentDesktop( int desktop )
{
    NETRootInfo info( QX11Info::display(), 0 );
    info.setCurrentDesktop( desktop );
}

void KWM::setOnAllDesktops( WId win, bool b )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), NET::WMDesktop );
    if ( b )
	info.setDesktop( NETWinInfo::OnAllDesktops );
    else if ( info.desktop()  == NETWinInfo::OnAllDesktops ) {
	NETRootInfo rinfo( QX11Info::display(), NET::CurrentDesktop );
	info.setDesktop( rinfo.currentDesktop() );
    }
}

void KWM::setOnDesktop( WId win, int desktop )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), NET::WMDesktop );
    info.setDesktop( desktop );
}

int KWM::numberOfViewports(int desktop)
{
    init( INFO_BASIC );
    return s_d_func()->numberOfViewports(desktop);
}

int KWM::currentViewport(int desktop)
{
    init( INFO_BASIC );
    return s_d_func()->currentViewport(desktop);
}

void KWM::setCurrentDesktopViewport( int desktop, QPoint viewport )
{
    NETRootInfo info( QX11Info::display(), 0 );
    NETPoint netview;
    netview.x = viewport.x();
    netview.y = viewport.y();
    info.setDesktopViewport( desktop, netview );
}

WId KWM::activeWindow()
{
    KWMPrivate* const s_d = s_d_func();
    if( s_d )
        return s_d->activeWindow();
    NETRootInfo info( QX11Info::display(), NET::ActiveWindow );
    return info.activeWindow();
}

void KWM::activateWindow( WId win, long time )
{
    NETRootInfo info( QX11Info::display(), 0 );
    if( time == 0 )
        time = QX11Info::appUserTime();
    info.setActiveWindow( win, NET::FromApplication, time,
        qApp->activeWindow() ? qApp->activeWindow()->winId() : 0 );
    KUniqueApplication::setHandleAutoStarted();
}

void KWM::forceActiveWindow( WId win, long time )
{
    NETRootInfo info( QX11Info::display(), 0 );
    if( time == 0 )
        time = QX11Info::appTime();
    info.setActiveWindow( win, NET::FromTool, time, 0 );
    KUniqueApplication::setHandleAutoStarted();
}

void KWM::demandAttention( WId win, bool set )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), 0 );
    info.setState( set ? NET::DemandsAttention : 0, NET::DemandsAttention );
}

WId KWM::transientFor( WId win )
{
    KXErrorHandler handler; // ignore badwindow
    Window transient_for = None;
    if( XGetTransientForHint( QX11Info::display(), win, &transient_for ))
        return transient_for;
    // XGetTransientForHint() did sync
    return None;
}

void KWM::setMainWindow( QWidget* subwindow, WId mainwindow )
{
    if( mainwindow != 0 )
    {
        /*
         Grmbl. See QDialog::show(). That should get fixed in Qt somehow.
        */
        if( qobject_cast< QDialog* >( subwindow ) != NULL
            && subwindow->parentWidget() == NULL
            && qApp->activeWindow() != NULL )
        {
            kWarning() << "KWM::setMainWindow(): There either mustn't be qApp->mainWidget(),"
                " or the dialog must have a non-NULL parent, otherwise Qt will reset the change. Bummer." << endl;
        }
        XSetTransientForHint( QX11Info::display(), subwindow->winId(), mainwindow );
    }
    else
        XDeleteProperty( QX11Info::display(), subwindow->winId(), XA_WM_TRANSIENT_FOR );
}

WId KWM::groupLeader( WId win )
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

QPixmap KWM::icon( WId win, int width, int height, bool scale )
{
    return icon( win, width, height, scale, NETWM | WMHints | ClassHint | XApp );
}


QPixmap KWM::icon( WId win, int width, int height, bool scale, int flags )
{
    KXErrorHandler handler; // ignore badwindow
    QPixmap result;
    if( flags & NETWM ) {
        NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), NET::WMIcon );
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
	    Window root;
	    int x, y;
	    unsigned int w = 0;
	    unsigned int h = 0;
            unsigned int border_w, depth;
	    XGetGeometry(QX11Info::display(), p, &root,
		         &x, &y, &w, &h, &border_w, &depth);
	    if (w > 0 && h > 0){
	        QPixmap pm(w, h);
	        // Always detach before doing something behind QPixmap's back.
	        pm.detach();
                GC gc = XCreateGC( QX11Info::display(), p, 0, NULL );
	        XCopyArea(QX11Info::display(), p, pm.handle(), gc,
		          0, 0, w, h, 0, 0);
                XFreeGC( QX11Info::display(), gc );
	        if (p_mask != None){
	    	    QBitmap bm(w, h);
                    GC gc = XCreateGC( QX11Info::display(), p_mask, 0, NULL );
		    XCopyArea(QX11Info::display(), p_mask, bm.handle(), gc,
			      0, 0, w, h, 0, 0);
		    pm.setMask(bm);
                    XFreeGC( QX11Info::display(), gc );
	        }
	        if ( scale && width > 0 && height > 0 && !pm.isNull() &&
		     ( (int) w != width || (int) h != height) ){
		    result = QPixmap::fromImage( pm.toImage().scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
	        } else {
		    result = pm;
	        }
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
	        QString className = hint.res_class;

                QPixmap pm = KIconLoader::global()->loadIcon( className.toLower(), K3Icon::Small, iconWidth,
                                                           K3Icon::DefaultState, 0, true );
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
	// If the icon is still a null pixmap, load the 'xapp' icon
	// as a last resort:
	if ( result.isNull() ) {
            QPixmap pm = KIconLoader::global()->loadIcon( "xapp", K3Icon::Small, iconWidth,
                                                          K3Icon::DefaultState, 0, true );
	    if( scale && !pm.isNull() )
		result = QPixmap::fromImage( pm.toImage().scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
	    else
		result = pm;
	}
    }
    return result;
}

void KWM::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon )
{
    if ( icon.isNull() )
	return;
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), 0 );
    QImage img = icon.toImage().convertToFormat( QImage::Format_ARGB32 );
    NETIcon ni;
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, true );
    if ( miniIcon.isNull() )
	return;
    img = miniIcon.toImage().convertToFormat( QImage::Format_ARGB32 );
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, false );
}

void KWM::setType( WId win, NET::WindowType windowType )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), 0 );
    info.setWindowType( windowType );
}

void KWM::setState( WId win, unsigned long state )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), NET::WMState );
    info.setState( state, state );
}

void KWM::clearState( WId win, unsigned long state )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), NET::WMState );
    info.setState( 0, state );
}

void KWM::minimizeWindow( WId win, bool animation)
{
    if ( !animation )
    {
        create_atoms();
	sendClientMessageToRoot( win, kde_wm_change_state, IconicState, 1 );
    }
	QX11Info inf;
    XIconifyWindow( QX11Info::display(), win, inf.screen() );
}

void KWM::unminimizeWindow( WId win, bool animation )
{
    if ( !animation )
    {
        create_atoms();
	sendClientMessageToRoot( win, kde_wm_change_state, NormalState, 1 );
    }
    XMapWindow( QX11Info::display(), win );
}

void KWM::raiseWindow( WId win )
{
    NETRootInfo info( QX11Info::display(), NET::Supported );
    if( info.isSupported( NET::WM2RestackWindow ))
        info.restackRequest( win, NET::FromTool, None, Above, QX11Info::appUserTime());
    else
        XRaiseWindow( QX11Info::display(), win );
}

void KWM::lowerWindow( WId win )
{
    NETRootInfo info( QX11Info::display(), NET::Supported );
    if( info.isSupported( NET::WM2RestackWindow ))
        info.restackRequest( win, NET::FromTool, None, Below, QX11Info::appUserTime());
    else
        XLowerWindow( QX11Info::display(), win );
}

bool KWM::compositingActive()
{
    create_atoms();
    return XGetSelectionOwner( QX11Info::display(), net_wm_cm ) != None;
}

QRect KWM::workArea( int desktop )
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

QRect KWM::workArea( const QList<WId>& exclude, int desktop )
{
    init( INFO_WINDOWS ); // invalidates s_d_func's return value
    KWMPrivate* const s_d = s_d_func();

    QRect all = QApplication::desktop()->geometry();
    QRect a = all;

    if (desktop == -1)
        desktop = s_d->currentDesktop();

    QList<WId>::ConstIterator it1;
    for( it1 = s_d->windows.begin(); it1 != s_d->windows.end(); ++it1 ) {

        if(exclude.contains(*it1))
            continue;

// Kicker (very) extensively calls this function, causing hundreds of roundtrips just
// to repeatedly find out struts of all windows. Therefore strut values for strut
// windows are cached here.
        NETStrut strut;
        QList< KWMPrivate::StrutData >::Iterator it2 = s_d->strutWindows.begin();
        for( ; it2 != s_d->strutWindows.end(); ++it2 )
            if( (*it2).window == *it1 )
                break;

            if( it2 != s_d->strutWindows.end()) {
                if(!((*it2).desktop == desktop || (*it2).desktop == NETWinInfo::OnAllDesktops ))
                    continue;

                strut = (*it2).strut;
            } else if( s_d->possibleStrutWindows.contains( *it1 ) ) {

                NETWinInfo info( QX11Info::display(), (*it1), QX11Info::appRootWindow(), NET::WMStrut | NET::WMDesktop);
                strut = info.strut();
                s_d->possibleStrutWindows.removeAll( *it1 );
                s_d->strutWindows.append( KWMPrivate::StrutData( *it1, info.strut(), info.desktop()));

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

        a = a.intersect(r);
    }
    return a;
}

QString KWM::desktopName( int desktop )
{
    init( INFO_BASIC );
    KWMPrivate* const s_d = s_d_func();

    bool isDesktopSane = (desktop > 0 && desktop <= (int) s_d->numberOfDesktops());
    const char* name = s_d->desktopName( isDesktopSane ? desktop : currentDesktop() );

    if ( name && name[0] )
        return QString::fromUtf8( name );

    return i18n("Desktop %1",  desktop );
}

void KWM::setDesktopName( int desktop, const QString& name )
{
    KWMPrivate* const s_d = s_d_func();

    if (desktop <= 0 || desktop > (int) numberOfDesktops() )
        desktop = currentDesktop();

    if( s_d ) {
        s_d->setDesktopName( desktop, name.toUtf8().data() );
        return;
    }

    NETRootInfo info( QX11Info::display(), 0 );
    info.setDesktopName( desktop, name.toUtf8().data() );
}

bool KWM::showingDesktop()
{
    init( INFO_BASIC );
    return s_d_func()->showingDesktop();
}

void KWM::setUserTime( WId win, long time )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), 0 );
    info.setUserTime( time );
}

void KWM::setExtendedStrut( WId win, int left_width, int left_start, int left_end,
    int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
    int bottom_width, int bottom_start, int bottom_end )
{
    NETWinInfo info( QX11Info::display(), win, QX11Info::appRootWindow(), 0 );
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

void KWM::setStrut( WId win, int left, int right, int top, int bottom )
{
    int w = XDisplayWidth( QX11Info::display(), DefaultScreen( QX11Info::display()));
    int h = XDisplayHeight( QX11Info::display(), DefaultScreen( QX11Info::display()));
    setExtendedStrut( win, left, 0, left != 0 ? w : 0, right, 0, right != 0 ? w : 0,
        top, 0, top != 0 ? h : 0, bottom, 0, bottom != 0 ? h : 0 );
}

bool KWM::icccmCompliantMappingState()
{
    static enum { noidea, yes, no } wm_is_1_2_compliant = noidea;
    if( wm_is_1_2_compliant == noidea ) {
        NETRootInfo info( QX11Info::display(), NET::Supported );
        wm_is_1_2_compliant = info.isSupported( NET::Hidden ) ? yes : no;
    }
    return wm_is_1_2_compliant == yes;
}

bool KWM::allowedActionsSupported()
{
    static enum { noidea, yes, no } wm_supports_allowed_actions = noidea;
    if( wm_supports_allowed_actions == noidea ) {
        NETRootInfo info( QX11Info::display(), NET::Supported );
        wm_supports_allowed_actions = info.isSupported( NET::WM2AllowedActions ) ? yes : no;
    }
    return wm_supports_allowed_actions == yes;
}

QString KWM::readNameProperty( WId win, unsigned long atom )
{
    XTextProperty tp;
    char **text = NULL;
    int count;
    QString result;
    if ( XGetTextProperty( QX11Info::display(), win, &tp, atom ) != 0 && tp.value != NULL ) {
        create_atoms();
        if ( XmbTextPropertyToTextList( QX11Info::display(), &tp, &text, &count) == Success &&
                  text != NULL && count > 0 ) {
            result = QString::fromLocal8Bit( text[0] );
        } else if ( tp.encoding == kwm_utf8_string ) {
            result = QString::fromUtf8 ( (const char*) tp.value );
        } else if ( tp.encoding == XA_STRING )
            result = QString::fromLocal8Bit( (const char*) tp.value );
        if( text != NULL )
            XFreeStringList( text );
        XFree( tp.value );
    }
    return result;
}

void KWM::doNotManage( const QString& title )
{
    QDBusInterface("org.kde.kwin", "/KWin", "org.kde.KWin", QDBusConnection::sessionBus())
        .call("doNotManage", title);
}


struct KWM::WindowInfo::Private
{
	Private()
	: info( NULL )
	{}
	~Private() { delete info; }
	NETWinInfo* info;
	WId win_;
	QString name_;
        QString iconic_name_;
	QRect geometry_;
        QRect frame_geometry_;
	int ref;
        bool valid;
    private:
	Private( const Private& );
	void operator=( const Private& );
};

// KWM::info() should be updated too if something has to be changed here
KWM::WindowInfo::WindowInfo( WId _win, unsigned long properties, unsigned long properties2 ) : d(new Private)
{
    KXErrorHandler handler;
    d->ref = 1;
    if( properties & NET::WMVisibleIconName )
	properties |= NET::WMIconName | NET::WMVisibleName; // force, in case it will be used as a fallback
    if( properties & NET::WMVisibleName )
	properties |= NET::WMName; // force, in case it will be used as a fallback
    if( properties2 & NET::WM2ExtendedStrut )
        properties |= NET::WMStrut; // will be used as fallback
    properties |= NET::XAWMState; // force to get error detection for valid()
    unsigned long props[ 2 ] = { properties, properties2 };
    d->info = new NETWinInfo( QX11Info::display(), _win, QX11Info::appRootWindow(), props, 2 );
    d->win_ = _win;
    if( properties & NET::WMName ) {
        if( d->info->name() && d->info->name()[ 0 ] != '\0' )
	    d->name_ = QString::fromUtf8( d->info->name() );
        else
            d->name_ = readNameProperty( _win, XA_WM_NAME );
    }
    if( properties & NET::WMIconName ) {
        if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
            d->iconic_name_ = QString::fromUtf8( d->info->iconName());
        else
            d->iconic_name_ = readNameProperty( _win, XA_WM_ICON_NAME );
    }
    if( properties & ( NET::WMGeometry | NET::WMKDEFrameStrut )) {
        NETRect frame, geom;
        d->info->kdeGeometry( frame, geom );
        d->geometry_.setRect( geom.pos.x, geom.pos.y, geom.size.width, geom.size.height );
        d->frame_geometry_.setRect( frame.pos.x, frame.pos.y, frame.size.width, frame.size.height );
    }
    d->valid = !handler.error( false ); // no sync - NETWinInfo did roundtrips
}

// this one is only to make QValueList<> or similar happy
KWM::WindowInfo::WindowInfo()
    : d( NULL )
{
}

KWM::WindowInfo::~WindowInfo()
{
    if( d != NULL ) {
	if( --d->ref == 0 ) {
	    delete d;
	}
    }
}

KWM::WindowInfo::WindowInfo( const WindowInfo& wininfo )
    : d( wininfo.d )
{
    if( d != NULL )
	++d->ref;
}

KWM::WindowInfo& KWM::WindowInfo::operator=( const WindowInfo& wininfo )
{
    if( d != wininfo.d ) {
	if( d != NULL )
	    if( --d->ref == 0 )
		delete d;
	d = wininfo.d;
	if( d != NULL )
	    ++d->ref;
    }
    return *this;
}

bool KWM::WindowInfo::valid( bool withdrawn_is_valid ) const
{
    if( !d->valid )
        return false;
    if( !withdrawn_is_valid && mappingState() == NET::Withdrawn )
        return false;
    return true;
}

WId KWM::WindowInfo::win() const
{
    return d->win_;
}

unsigned long KWM::WindowInfo::state() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMState ) == 0, 176 )
        << "Pass NET::WMState to KWM::windowInfo()" << endl;
    return d->info->state();
}

NET::MappingState KWM::WindowInfo::mappingState() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::XAWMState ) == 0, 176 )
        << "Pass NET::XAWMState to KWM::windowInfo()" << endl;
    return d->info->mappingState();
}

NETExtendedStrut KWM::WindowInfo::extendedStrut() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ExtendedStrut ) == 0, 176 )
        << "Pass NET::WM2ExtendedStrut to second argument of KWM::windowInfo()" << endl;
    NETExtendedStrut ext = d->info->extendedStrut();
    NETStrut str = d->info->strut();
    if( ext.left_width == 0 && ext.right_width == 0 && ext.top_width == 0 && ext.bottom_width == 0
        && ( str.left != 0 || str.right != 0 || str.top != 0 || str.bottom != 0 )) {
        // build extended from simple
        if( str.left != 0 ) {
            ext.left_width = str.left;
            ext.left_start = 0;
            ext.left_end = XDisplayHeight( QX11Info::display(), DefaultScreen( QX11Info::display()));
        }
        if( str.right != 0 ) {
            ext.right_width = str.right;
            ext.right_start = 0;
            ext.right_end = XDisplayHeight( QX11Info::display(), DefaultScreen( QX11Info::display()));
        }
        if( str.top != 0 ) {
            ext.top_width = str.top;
            ext.top_start = 0;
            ext.top_end = XDisplayWidth( QX11Info::display(), DefaultScreen( QX11Info::display()));
        }
        if( str.bottom != 0 ) {
            ext.bottom_width = str.bottom;
            ext.bottom_start = 0;
            ext.bottom_end = XDisplayWidth( QX11Info::display(), DefaultScreen( QX11Info::display()));
        }
    }
    return ext;
}

NET::WindowType KWM::WindowInfo::windowType( int supported_types ) const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMWindowType ) == 0, 176 )
        << "Pass NET::WMWindowType to KWM::windowInfo()" << endl;
    return d->info->windowType( supported_types );
}

QString KWM::WindowInfo::visibleNameWithState() const
{
    QString s = visibleName();
    if ( isMinimized() ) {
	s.prepend(QLatin1Char('('));
	s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWM::WindowInfo::visibleName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleName ) == 0, 176 )
        << "Pass NET::WMVisibleName to KWM::windowInfo()" << endl;
    return d->info->visibleName() && d->info->visibleName()[ 0 ] != '\0'
        ? QString::fromUtf8(d->info->visibleName()) : name();
}

QString KWM::WindowInfo::name() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMName ) == 0, 176 )
        << "Pass NET::WMName to KWM::windowInfo()" << endl;
    return d->name_;
}

QString KWM::WindowInfo::visibleIconNameWithState() const
{
    QString s = visibleIconName();
    if ( isMinimized() ) {
	s.prepend(QLatin1Char('('));
	s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWM::WindowInfo::visibleIconName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleIconName ) == 0, 176 )
        << "Pass NET::WMVisibleIconName to KWM::windowInfo()" << endl;
    if( d->info->visibleIconName() && d->info->visibleIconName()[ 0 ] != '\0' )
        return QString::fromUtf8( d->info->visibleIconName());
    if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
        return QString::fromUtf8( d->info->iconName());
    if( !d->iconic_name_.isEmpty())
        return d->iconic_name_;
    return visibleName();
}

QString KWM::WindowInfo::iconName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMIconName ) == 0, 176 )
        << "Pass NET::WMIconName to KWM::windowInfo()" << endl;
    if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
        return QString::fromUtf8( d->info->iconName());
    if( !d->iconic_name_.isEmpty())
        return d->iconic_name_;
    return name();
}

bool KWM::WindowInfo::isOnCurrentDesktop() const
{
    return isOnDesktop( KWM::currentDesktop());
}

bool KWM::WindowInfo::isOnDesktop( int _desktop ) const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWM::windowInfo()" << endl;
    return d->info->desktop() == _desktop || d->info->desktop() == NET::OnAllDesktops;
}

bool KWM::WindowInfo::onAllDesktops() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWM::windowInfo()" << endl;
    return d->info->desktop() == NET::OnAllDesktops;
}

int KWM::WindowInfo::desktop() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWM::windowInfo()" << endl;
    return d->info->desktop();
}

QRect KWM::WindowInfo::geometry() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMGeometry ) == 0, 176 )
        << "Pass NET::WMGeometry to KWM::windowInfo()" << endl;
    return d->geometry_;
}

QRect KWM::WindowInfo::frameGeometry() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMKDEFrameStrut ) == 0, 176 )
        << "Pass NET::WMKDEFrameStrut to KWM::windowInfo()" << endl;
    return d->frame_geometry_;
}

WId KWM::WindowInfo::transientFor() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2TransientFor ) == 0, 176 )
        << "Pass NET::WM2TransientFor to KWM::windowInfo()" << endl;
    return d->info->transientFor();
}

WId KWM::WindowInfo::groupLeader() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2GroupLeader ) == 0, 176 )
        << "Pass NET::WM2GroupLeader to KWM::windowInfo()" << endl;
    return d->info->groupLeader();
}

QByteArray KWM::WindowInfo::windowClassClass() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWM::windowInfo()" << endl;
    return d->info->windowClassClass();
}

QByteArray KWM::WindowInfo::windowClassName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWM::windowInfo()" << endl;
    return d->info->windowClassName();
}

QByteArray KWM::WindowInfo::windowRole() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowRole ) == 0, 176 )
        << "Pass NET::WM2WindowRole to KWM::windowInfo()" << endl;
    return d->info->windowRole();
}

QByteArray KWM::WindowInfo::clientMachine() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ClientMachine ) == 0, 176 )
        << "Pass NET::WM2ClientMachine to KWM::windowInfo()" << endl;
    return d->info->clientMachine();
}

bool KWM::WindowInfo::actionSupported( NET::Action action ) const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2AllowedActions ) == 0, 176 )
        << "Pass NET::WM2AllowedActions to KWM::windowInfo()" << endl;
    if( allowedActionsSupported())
        return d->info->allowedActions() & action;
    else
        return true; // no idea if it's supported or not -> pretend it is
}

// see NETWM spec section 7.6
bool KWM::WindowInfo::isMinimized() const
{
    if( mappingState() != NET::Iconic )
        return false;
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if(( state() & NET::Hidden ) != 0
	&& ( state() & NET::Shaded ) == 0 ) // shaded may have NET::Hidden too
        return true;
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return icccmCompliantMappingState() ? false : true;
}

#include "kwm.moc"
