/*
    $Id: kwinmodule.cpp 629176 2007-02-01 18:55:16Z dfaure $

    This file is part of the KDE libraries
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

#include <qwidget.h>
#ifdef Q_WS_X11 //FIXME
#include "kwinmodule.h"
#include "kwin.h"
#include <X11/Xatom.h>
#include "kapplication.h"
#include "kdebug.h"
#include <qtl.h>
#include <qptrlist.h>
#include <klocale.h>
#include <dcopclient.h>
#include "netwm.h"

static KWinModulePrivate* static_d = 0;

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

static unsigned long desktop_properties[ 2 ] = { 
				     NET::NumberOfDesktops |
				     NET::DesktopGeometry |
                                     NET::DesktopViewport |
				     NET::CurrentDesktop |
				     NET::DesktopNames |
				     NET::ActiveWindow |
				     NET::WorkArea |
				     NET::KDESystemTrayWindows,
                                     NET::WM2ShowingDesktop };

class KWinModulePrivate : public QWidget, public NETRootInfo4
{
public:
    KWinModulePrivate(int _what)
	: QWidget(0,0), NETRootInfo4( qt_xdisplay(),
                                     _what >= KWinModule::INFO_WINDOWS ?
                                     windows_properties : desktop_properties,
                                     2,
				     -1, false
				     ),
          strutSignalConnected( false ),
          what( _what )
    {
	kapp->installX11EventFilter( this );
	(void ) kapp->desktop(); //trigger desktop widget creation to select root window events
        activate();
	updateStackingOrder();
    }
    ~KWinModulePrivate()
    {
    }
    QPtrList<KWinModule> modules;

    QValueList<WId> windows;
    QValueList<WId> stackingOrder;
    QValueList<WId> systemTrayWindows;

    struct StrutData
    {
        StrutData( WId window_, const NETStrut& strut_, int desktop_ )
            : window( window_ ), strut( strut_ ), desktop( desktop_ ) {};
        StrutData() {}; // for QValueList to be happy
        WId window;
        NETStrut strut;
        int desktop;
    };
    QValueList<StrutData> strutWindows;
    QValueList<WId> possibleStrutWindows;
    bool strutSignalConnected;
    int what;

    void addClient(Window);
    void removeClient(Window);
    void addSystemTrayWin(Window);
    void removeSystemTrayWin(Window);

    bool x11Event( XEvent * ev );

    void updateStackingOrder();
    bool removeStrutWindow( WId );

    QSize numberOfViewports(int desktop) const;
    QPoint currentViewport(int desktop) const;
};

KWinModule::KWinModule( QObject* parent )
    : QObject( parent, "kwin_module" )
{
    init(INFO_ALL);
}

KWinModule::KWinModule( QObject* parent, int what )
    : QObject( parent, "kwin_module" )
{
    init(what);
}

void KWinModule::init(int what)
{
    if (what >= INFO_WINDOWS)
       what = INFO_WINDOWS;
    else
       what = INFO_DESKTOP;

    if ( !static_d )
    {
        static_d = new KWinModulePrivate(what);
    }
    else if (static_d->what < what)
    {
        QPtrList<KWinModule> modules = static_d->modules;
        delete static_d;
        static_d = new KWinModulePrivate(what);
        static_d->modules = modules;
        for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
            (*mit)->d = static_d;
    }
    
    d = static_d;
    d->modules.append( this );
}

KWinModule::~KWinModule()
{
    d->modules.removeRef( this );
    if ( d->modules.isEmpty() ) {
	delete d;
	static_d = 0;
    }
}

const QValueList<WId>& KWinModule::windows() const
{
    return d->windows;
}

const QValueList<WId>& KWinModule::stackingOrder() const
{
    return d->stackingOrder;
}


bool KWinModule::hasWId(WId w) const
{
    return d->windows.findIndex( w ) != -1;
}

const QValueList<WId>& KWinModule::systemTrayWindows() const
{
    return d->systemTrayWindows;
}

QSize KWinModulePrivate::numberOfViewports(int desktop) const
{
    NETSize netdesktop = desktopGeometry(desktop);
    QSize s(netdesktop.width / QApplication::desktop()->width(),
            netdesktop.height / QApplication::desktop()->height());

    // workaround some kwin bugs
    if (s.width() < 1) s.setWidth(1);
    if (s.height() < 1) s.setHeight(1);
    return s;
}

QPoint KWinModulePrivate::currentViewport(int desktop) const
{
    NETPoint netviewport = desktopViewport(desktop);

    return QPoint(1+(netviewport.x / QApplication::desktop()->width()),
            1+(netviewport.y / QApplication::desktop()->height()));
}

bool KWinModulePrivate::x11Event( XEvent * ev )
{
    if ( ev->xany.window == qt_xrootwin() ) {
        int old_current_desktop = currentDesktop();
        WId old_active_window = activeWindow();
        int old_number_of_desktops = numberOfDesktops();
        bool old_showing_desktop = showingDesktop();
        unsigned long m[ 5 ];
	NETRootInfo::event( ev, m, 5 );

	if (( m[ PROTOCOLS ] & CurrentDesktop ) && currentDesktop() != old_current_desktop )
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->currentDesktopChanged( currentDesktop() );
	if (( m[ PROTOCOLS ] & ActiveWindow ) && activeWindow() != old_active_window )
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->activeWindowChanged( activeWindow() );
	if ( m[ PROTOCOLS ] & DesktopViewport ) {
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->currentDesktopViewportChanged(currentDesktop(),
                        currentViewport(currentDesktop()));
        }
	if ( m[ PROTOCOLS ] & DesktopGeometry ) {
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->desktopGeometryChanged(currentDesktop());
	}
	if ( m[ PROTOCOLS ] & DesktopNames )
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->desktopNamesChanged();
	if (( m[ PROTOCOLS ] & NumberOfDesktops ) && numberOfDesktops() != old_number_of_desktops )
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->numberOfDesktopsChanged( numberOfDesktops() );
	if ( m[ PROTOCOLS ] & WorkArea )
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->workAreaChanged();
	if ( m[ PROTOCOLS ] & ClientListStacking ) {
	    updateStackingOrder();
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->stackingOrderChanged();
	}
        if(( m[ PROTOCOLS2 ] & WM2ShowingDesktop ) && showingDesktop() != old_showing_desktop ) {
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
		emit (*mit)->showingDesktopChanged( showingDesktop());
        }
    } else  if ( windows.findIndex( ev->xany.window ) != -1 ){
	NETWinInfo ni( qt_xdisplay(), ev->xany.window, qt_xrootwin(), 0 );
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
            if ( possibleStrutWindows.findIndex( ev->xany.window ) == -1 )
        	possibleStrutWindows.append( ev->xany.window );
	}
	if ( dirty[ NETWinInfo::PROTOCOLS ] || dirty[ NETWinInfo::PROTOCOLS2 ] ) {
	    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit ) {
		emit (*mit)->windowChanged( ev->xany.window );
                emit (*mit)->windowChanged( ev->xany.window, dirty );
		emit (*mit)->windowChanged( ev->xany.window, dirty[ NETWinInfo::PROTOCOLS ] );
		if ( (dirty[ NETWinInfo::PROTOCOLS ] & NET::WMStrut) != 0 )
		    emit (*mit)->strutChanged();
	    }
	}
    }

    return false;
}

bool KWinModulePrivate::removeStrutWindow( WId w )
{
    for( QValueList< StrutData >::Iterator it = strutWindows.begin();
         it != strutWindows.end();
         ++it )
        if( (*it).window == w ) {
            strutWindows.remove( it );
            return true;
        }
    return false;
}

void KWinModulePrivate::updateStackingOrder()
{
    stackingOrder.clear();
    for ( int i = 0; i <  clientListStackingCount(); i++ )
	stackingOrder.append( clientListStacking()[i] );
}

void KWinModulePrivate::addClient(Window w)
{
    if ( (what >= KWinModule::INFO_WINDOWS) && !QWidget::find( w ) )
	XSelectInput( qt_xdisplay(), w, PropertyChangeMask | StructureNotifyMask );
    bool emit_strutChanged = false;
    if( strutSignalConnected && modules.count() > 0 ) {
        NETWinInfo info( qt_xdisplay(), w, qt_xrootwin(), NET::WMStrut | NET::WMDesktop );
        NETStrut strut = info.strut();
        if ( strut.left || strut.top || strut.right || strut.bottom ) {
            strutWindows.append( StrutData( w, strut, info.desktop()));
            emit_strutChanged = true;
        }
    } else
        possibleStrutWindows.append( w );
    windows.append( w );
    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit ) {
	emit (*mit)->windowAdded( w );
	if ( emit_strutChanged )
	    emit (*mit)->strutChanged();
    }
}

void KWinModulePrivate::removeClient(Window w)
{
    bool emit_strutChanged = removeStrutWindow( w );
    if( strutSignalConnected && possibleStrutWindows.findIndex( w ) != -1 && modules.count() > 0 ) {
        NETWinInfo info( qt_xdisplay(), w, qt_xrootwin(), NET::WMStrut );
        NETStrut strut = info.strut();
        if ( strut.left || strut.top || strut.right || strut.bottom ) {
            emit_strutChanged = true;
        }
    }
    possibleStrutWindows.remove( w );
    windows.remove( w );
    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit ) {
	emit (*mit)->windowRemoved( w );
	if ( emit_strutChanged )
	    emit (*mit)->strutChanged();
    }
}

void KWinModulePrivate::addSystemTrayWin(Window w)
{
    systemTrayWindows.append( w );
    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
	emit (*mit)->systemTrayWindowAdded( w );
}

void KWinModulePrivate::removeSystemTrayWin(Window w)
{
    systemTrayWindows.remove( w );
    for ( QPtrListIterator<KWinModule> mit( modules ); mit.current(); ++mit )
	emit (*mit)->systemTrayWindowRemoved( w );
}

int KWinModule::currentDesktop() const
{
    return d->currentDesktop();
}

int KWinModule::numberOfDesktops() const
{
    return d->numberOfDesktops();
}

QSize KWinModule::numberOfViewports(int desktop) const
{
    return d->numberOfViewports(desktop);
}

QPoint KWinModule::currentViewport(int desktop) const
{
    return d->currentViewport(desktop);
}

WId KWinModule::activeWindow() const
{
    return d->activeWindow();
}

bool KWinModule::showingDesktop() const
{
    return d->showingDesktop();
}

QRect KWinModule::workArea( int desktop ) const
{
    int desk  = (desktop > 0 && desktop <= (int) d->numberOfDesktops() ) ? desktop : currentDesktop();
    if ( desk <= 0 )
	return QApplication::desktop()->geometry();
    NETRect r = d->workArea( desk );
    if( r.size.width <= 0 || r.size.height <= 0 ) // not set
	return QApplication::desktop()->geometry();
    return QRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
}

QRect KWinModule::workArea( const QValueList<WId>& exclude, int desktop ) const
{
    QRect all = QApplication::desktop()->geometry();
    QRect a = all;

    if (desktop == -1)
	desktop = d->currentDesktop();

    QValueList<WId>::ConstIterator it1;
    for( it1 = d->windows.begin(); it1 != d->windows.end(); ++it1 ) {

	if(exclude.findIndex(*it1) != -1) continue;
        
// Kicker (very) extensively calls this function, causing hundreds of roundtrips just
// to repeatedly find out struts of all windows. Therefore strut values for strut
// windows are cached here.
        NETStrut strut;
        QValueList< KWinModulePrivate::StrutData >::Iterator it2 = d->strutWindows.begin();
        for( ;
             it2 != d->strutWindows.end();
             ++it2 )
            if( (*it2).window == *it1 )
                break;
        if( it2 != d->strutWindows.end()) {
            if(!((*it2).desktop == desktop || (*it2).desktop == NETWinInfo::OnAllDesktops ))
                continue;
            strut = (*it2).strut;
        } else if( d->possibleStrutWindows.findIndex( *it1 ) != -1 ) {
            NETWinInfo info( qt_xdisplay(), (*it1), qt_xrootwin(), NET::WMStrut | NET::WMDesktop);
	    strut = info.strut();
            d->possibleStrutWindows.remove( *it1 );
            d->strutWindows.append( KWinModulePrivate::StrutData( *it1, info.strut(), info.desktop()));
	    if(!(info.desktop() == desktop || info.desktop() == NETWinInfo::OnAllDesktops))
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

void KWinModule::connectNotify( const char* signal )
{
    if( !d->strutSignalConnected && qstrcmp( signal, SIGNAL(strutChanged())) == 0 )
        d->strutSignalConnected = true;
    QObject::connectNotify( signal );
}

QString KWinModule::desktopName( int desktop ) const
{
    const char* name = d->desktopName( (desktop > 0 && desktop <= (int) d->numberOfDesktops() ) ? desktop : currentDesktop() );
    if ( name && name[0] )
	return QString::fromUtf8( name );
    return i18n("Desktop %1").arg( desktop );
}

void KWinModule::setDesktopName( int desktop, const QString& name )
{
    if (desktop <= 0 || desktop > (int) d->numberOfDesktops() )
	desktop = currentDesktop();
    d->setDesktopName( desktop, name.utf8().data() );
}


void KWinModule::doNotManage( const QString& title )
{
    if ( !kapp->dcopClient()->isAttached() )
	kapp->dcopClient()->attach();
    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly);
    arg << title;
    kapp->dcopClient()->call("kwin", "", "doNotManage(QString)",
			     data, replyType, replyData);
}

#include "kwinmodule.moc"
#endif
