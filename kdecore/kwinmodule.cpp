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

#include "kwinmodule.h"
#include "kwin.h"
#include <X11/Xatom.h>
#include "kapp.h"
#include <qtl.h>
#include "netwm.h"

extern Atom net_wm_context_help;
extern void kwin_net_create_atoms();

class KWinModulePrivate : public QWidget, public NETRootInfo
{
public:
    KWinModulePrivate( KWinModule* m  )
	: QWidget(0,0), NETRootInfo( qt_xdisplay(),
				     ClientList |
				     ClientListStacking |
				     NumberOfDesktops |
				     DesktopGeometry |
				     CurrentDesktop |
				     DesktopNames |
				     ActiveWindow |
				     WorkArea |
				     KDEDockingWindows
				     )
    {
	kwin_net_create_atoms();
	module = m;
	kapp->installX11EventFilter( this );
	(void ) kapp->desktop(); //trigger desktop widget creation to select root window events
	updateStackingOrder();
    }
    ~KWinModulePrivate()
    {
    }

    KWinModule* module;

    QValueList<WId> windows;
    QValueList<WId> stackingOrder;
    QValueList<WId> dockWindows;

    void addClient(Window);
    void removeClient(Window);
    void addDockWin(Window);
    void removeDockWin(Window);

    bool x11Event( XEvent * ev );

    void updateStackingOrder();
};


KWinModule::KWinModule( QObject* parent )
    : QObject( parent, "kwin_module" )
{
    d = new KWinModulePrivate( this );
    d->activate();
}

KWinModule::~KWinModule()
{
    delete d;
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

const QValueList<WId>& KWinModule::dockWindows() const
{
    return d->dockWindows;
}

bool KWinModulePrivate::x11Event( XEvent * ev )
{
    if ( ev->xany.window == qt_xrootwin() ) {
	int m = NETRootInfo::event( ev );

	if ( m & CurrentDesktop )
	    emit module->currentDesktopChanged( currentDesktop() );
	if ( m & ActiveWindow )
	    emit module->activeWindowChanged( activeWindow() );
	if ( m & DesktopNames )
	    emit module->desktopNamesChanged();
	if ( m & NumberOfDesktops )
	    emit module->numberOfDesktopsChanged( numberOfDesktops() );
	if ( m & WorkArea )
	    emit module->workAreaChanged();
	if ( m & ClientListStacking )
	    emit module->stackingOrderChanged();
    } else  if ( windows.contains( ev->xany.window ) ){
	NETWinInfo ni( qt_xdisplay(), ev->xany.window, qt_xrootwin(), 0 );
	unsigned int dirty = ni.event( ev );
	if ( !dirty && ev->type ==PropertyNotify && ev->xproperty.atom == XA_WM_HINTS )
	    dirty |= NET::WMIcon; // support for old icons
	emit module->windowChanged( ev->xany.window );
	emit module->windowChanged( ev->xany.window, dirty );
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
	XSelectInput( qt_xdisplay(), w, PropertyChangeMask );
    windows.append( w );
    emit module->windowAdded( w );
}

void KWinModulePrivate::removeClient(Window w)
{
    windows.remove( w );
    emit module->windowRemoved( w );
}
void KWinModulePrivate::addDockWin(Window w)
{
    dockWindows.append( w );
    emit module->dockWindowAdded( w );
}

void KWinModulePrivate::removeDockWin(Window w)
{
    dockWindows.remove( w );
    emit module->dockWindowRemoved( w );
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
    NETRect r = d->workArea( (desktop > 0 && desktop <= (int) d->numberOfDesktops() ) ? desktop-1 : currentDesktop()-1 );
    return QRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
}


#include "kwinmodule.moc"
