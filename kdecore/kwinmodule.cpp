/*
    $Id$

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qwidget.h>
#ifdef Q_WS_X11 //FIXME
#include "kwinmodule.h"
#include "kwin.h"
#include <X11/Xatom.h>
#include "kapp.h"
#include "kdebug.h"
#include <qtl.h>
#include <qlist.h>
#include <klocale.h>
#include <dcopclient.h>
#include "netwm.h"

extern Atom net_wm_context_help;
extern void kwin_net_create_atoms();

static KWinModulePrivate* static_d = 0;

class KWinModulePrivate : public QWidget, public NETRootInfo
{
public:
    KWinModulePrivate()
	: QWidget(0,0), NETRootInfo( qt_xdisplay(),
				     ClientList |
				     ClientListStacking |
				     NumberOfDesktops |
				     DesktopGeometry |
				     CurrentDesktop |
				     DesktopNames |
				     ActiveWindow |
				     WorkArea |
				     KDESystemTrayWindows,
				     -1, false
				     )
    {
	kwin_net_create_atoms();
	kapp->installX11EventFilter( this );
	(void ) kapp->desktop(); //trigger desktop widget creation to select root window events
	updateStackingOrder();
    }
    ~KWinModulePrivate()
    {
    }

    QPtrList<KWinModule> modules;
    KWinModule* module;

    QValueList<WId> windows;
    QValueList<WId> stackingOrder;
    QValueList<WId> systemTrayWindows;

    QValueList<WId> strutWindows;

    void addClient(Window);
    void removeClient(Window);
    void addSystemTrayWin(Window);
    void removeSystemTrayWin(Window);

    bool x11Event( XEvent * ev );

    void updateStackingOrder();
};


KWinModule::KWinModule( QObject* parent )
    : QObject( parent, "kwin_module" )
{
    if ( !static_d ) {
	static_d = new KWinModulePrivate;
	static_d->activate();
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
    return d->windows.contains( w );
}

const QValueList<WId>& KWinModule::systemTrayWindows() const
{
    return d->systemTrayWindows;
}

bool KWinModulePrivate::x11Event( XEvent * ev )
{
    if ( ev->xany.window == qt_xrootwin() ) {
	int m = NETRootInfo::event( ev );

	if ( m & CurrentDesktop )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->currentDesktopChanged( currentDesktop() );
	if ( m & ActiveWindow )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->activeWindowChanged( activeWindow() );
	if ( m & DesktopNames )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->desktopNamesChanged();
	if ( m & NumberOfDesktops )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->numberOfDesktopsChanged( numberOfDesktops() );
	if ( m & WorkArea )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->workAreaChanged();
	if ( m & ClientListStacking ) {
	    updateStackingOrder();
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->stackingOrderChanged();
	}
    } else  if ( windows.contains( ev->xany.window ) ){
	NETWinInfo ni( qt_xdisplay(), ev->xany.window, qt_xrootwin(), 0 );
	unsigned int dirty = ni.event( ev );
	if ( !dirty && ev->type ==PropertyNotify && ev->xproperty.atom == XA_WM_HINTS )
	    dirty |= NET::WMIcon; // support for old icons
	if ( (dirty & NET::WMStrut) != 0 ) {
	    if ( !strutWindows.contains( ev->xany.window )  )
		strutWindows.append( ev->xany.window );
	}
	if ( dirty ) {
	    for ( module = modules.first(); module; module = modules.next() ) {
		emit module->windowChanged( ev->xany.window );
		emit module->windowChanged( ev->xany.window, dirty );
		if ( (dirty & NET::WMStrut) != 0 )
		    emit module->strutChanged();
	    }
	}
    }

    return FALSE;
}


void KWinModulePrivate::updateStackingOrder()
{
    stackingOrder.clear();
    for ( int i = 0; i <  clientListStackingCount(); i++ )
	stackingOrder.append( clientListStacking()[i] );
}

void KWinModulePrivate::addClient(Window w)
{
    if ( !QWidget::find( w ) )
	XSelectInput( qt_xdisplay(), w, PropertyChangeMask | StructureNotifyMask );
    bool emit_strutChanged = FALSE;
    for ( module = modules.first(); module; module = modules.next() ) {
	NETWinInfo info( qt_xdisplay(), w, qt_xrootwin(), NET::WMStrut );
	NETStrut strut = info.strut();
	if ( strut.left || strut.top || strut.right || strut.bottom ) {
	    strutWindows.append( w );
	    emit_strutChanged = TRUE;
	}
	break;
    }
    windows.append( w );
    for ( module = modules.first(); module; module = modules.next() ) {
	emit module->windowAdded( w );
	if ( emit_strutChanged )
	    emit module->strutChanged();
    }
}

void KWinModulePrivate::removeClient(Window w)
{
    bool emit_strutChanged = strutWindows.contains( w );
    strutWindows.remove( w );
    windows.remove( w );
    for ( module = modules.first(); module; module = modules.next() ) {
	emit module->windowRemoved( w );
	if ( emit_strutChanged )
	    emit module->strutChanged();
    }
}

void KWinModulePrivate::addSystemTrayWin(Window w)
{
    systemTrayWindows.append( w );
    for ( module = modules.first(); module; module = modules.next() )
	emit module->systemTrayWindowAdded( w );
}

void KWinModulePrivate::removeSystemTrayWin(Window w)
{
    systemTrayWindows.remove( w );
    for ( module = modules.first(); module; module = modules.next() )
	emit module->systemTrayWindowRemoved( w );
}

int KWinModule::currentDesktop() const
{
    return d->currentDesktop();
}

int KWinModule::numberOfDesktops() const
{
    return d->numberOfDesktops();
}

WId KWinModule::activeWindow() const
{
    return d->activeWindow();
}

QRect KWinModule::workArea( int desktop ) const
{
    int desk  = (desktop > 0 && desktop <= (int) d->numberOfDesktops() ) ? desktop : currentDesktop();
    if ( desk <= 0 )
	return QApplication::desktop()->geometry();
    NETRect r = d->workArea( desk );
    return QRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
}

QRect KWinModule::workArea( const QValueList<WId>& exclude, int desktop ) const
{
    QRect all = QApplication::desktop()->geometry();
    QRect a = all;

    if (desktop == -1)
	desktop = d->currentDesktop();

    QValueList<WId>::ConstIterator it;
    for( it = d->windows.begin(); it != d->windows.end(); ++it ) {

	if(exclude.contains(*it) > 0) continue;

	NETWinInfo info( qt_xdisplay(), (*it), qt_xrootwin(), NET::WMStrut | NET::WMDesktop);
	//if(!(info.desktop() == desktop || info.desktop() == NETWinInfo::OnAllDesktops)) continue;

	QRect r = all;
	NETStrut strut = info.strut();
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
