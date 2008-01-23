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

#include "kwindowinfo.h"
#include "kwindowsystem.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kuniqueapplication.h>
#include <kdebug.h>
#include <kxerrorhandler.h>
#include <netwm.h>
#include <QtGui/QBitmap>
#include <QDesktopWidget>
#include <QtGui/QDialog>
#include <QtDBus/QtDBus>
#include <QtGui/QX11Info>
#include <X11/Xatom.h>

struct KWindowInfo::Private
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

// KWindowSystem::info() should be updated too if something has to be changed here
KWindowInfo::KWindowInfo( WId _win, unsigned long properties, unsigned long properties2 ) : d(new Private)
{
    KXErrorHandler handler;
    d->ref = 1;
    if( properties & NET::WMVisibleIconName )
	properties |= NET::WMIconName | NET::WMVisibleName; // force, in case it will be used as a fallback
    if( properties & NET::WMVisibleName )
	properties |= NET::WMName; // force, in case it will be used as a fallback
    if( properties2 & NET::WM2ExtendedStrut )
        properties |= NET::WMStrut; // will be used as fallback
    if( properties & NET::WMWindowType )
        properties2 |= NET::WM2TransientFor; // will be used when type is not set
    properties |= NET::XAWMState; // force to get error detection for valid()
    unsigned long props[ 2 ] = { properties, properties2 };
    d->info = new NETWinInfo( QX11Info::display(), _win, QX11Info::appRootWindow(), props, 2 );
    d->win_ = _win;
    if( properties & NET::WMName ) {
        if( d->info->name() && d->info->name()[ 0 ] != '\0' )
	    d->name_ = QString::fromUtf8( d->info->name() );
        else
            d->name_ = KWindowSystem::readNameProperty( _win, XA_WM_NAME );
    }
    if( properties & NET::WMIconName ) {
        if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
            d->iconic_name_ = QString::fromUtf8( d->info->iconName());
        else
            d->iconic_name_ = KWindowSystem::readNameProperty( _win, XA_WM_ICON_NAME );
    }
    if( properties & ( NET::WMGeometry | NET::WMFrameExtents )) {
        NETRect frame, geom;
        d->info->kdeGeometry( frame, geom );
        d->geometry_.setRect( geom.pos.x, geom.pos.y, geom.size.width, geom.size.height );
        d->frame_geometry_.setRect( frame.pos.x, frame.pos.y, frame.size.width, frame.size.height );
    }
    d->valid = !handler.error( false ); // no sync - NETWinInfo did roundtrips
}

// this one is only to make QList<> or similar happy
KWindowInfo::KWindowInfo()
    : d( NULL )
{
}

KWindowInfo::~KWindowInfo()
{
    if( d != NULL ) {
	if( --d->ref == 0 ) {
	    delete d;
	}
    }
}

KWindowInfo::KWindowInfo( const KWindowInfo& wininfo )
    : d( wininfo.d )
{
    if( d != NULL )
	++d->ref;
}

KWindowInfo& KWindowInfo::operator=( const KWindowInfo& wininfo )
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

bool KWindowInfo::valid( bool withdrawn_is_valid ) const
{
    if( !d->valid )
        return false;
    if( !withdrawn_is_valid && mappingState() == NET::Withdrawn )
        return false;
    return true;
}

WId KWindowInfo::win() const
{
    return d->win_;
}

unsigned long KWindowInfo::state() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMState ) == 0, 176 )
        << "Pass NET::WMState to KWindowInfo" << endl;
    return d->info->state();
}

bool KWindowInfo::hasState( unsigned long s ) const
{
    return ( state() & s ) == s;
}

NET::MappingState KWindowInfo::mappingState() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::XAWMState ) == 0, 176 )
        << "Pass NET::XAWMState to KWindowInfo" << endl;
    return d->info->mappingState();
}

NETExtendedStrut KWindowInfo::extendedStrut() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ExtendedStrut ) == 0, 176 )
        << "Pass NET::WM2ExtendedStrut to second argument of KWindowInfo" << endl;
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

NET::WindowType KWindowInfo::windowType( int supported_types ) const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMWindowType ) == 0, 176 )
        << "Pass NET::WMWindowType to KWindowInfo" << endl;
    if( !d->info->hasWindowType()) { // fallback, per spec recommendation
        if( transientFor() != None ) { // dialog
            if( supported_types & NET::DialogMask )
                return NET::Dialog;
        } else {
            if( supported_types & NET::NormalMask )
                return NET::Normal;
        }
    }
    return d->info->windowType( supported_types );
}

QString KWindowInfo::visibleNameWithState() const
{
    QString s = visibleName();
    if ( isMinimized() ) {
	s.prepend(QLatin1Char('('));
	s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfo::visibleName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleName ) == 0, 176 )
        << "Pass NET::WMVisibleName to KWindowInfo" << endl;
    return d->info->visibleName() && d->info->visibleName()[ 0 ] != '\0'
        ? QString::fromUtf8(d->info->visibleName()) : name();
}

QString KWindowInfo::name() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMName ) == 0, 176 )
        << "Pass NET::WMName to KWindowInfo" << endl;
    return d->name_;
}

QString KWindowInfo::visibleIconNameWithState() const
{
    QString s = visibleIconName();
    if ( isMinimized() ) {
	s.prepend(QLatin1Char('('));
	s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfo::visibleIconName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleIconName ) == 0, 176 )
        << "Pass NET::WMVisibleIconName to KWindowInfo" << endl;
    if( d->info->visibleIconName() && d->info->visibleIconName()[ 0 ] != '\0' )
        return QString::fromUtf8( d->info->visibleIconName());
    if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
        return QString::fromUtf8( d->info->iconName());
    if( !d->iconic_name_.isEmpty())
        return d->iconic_name_;
    return visibleName();
}

QString KWindowInfo::iconName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMIconName ) == 0, 176 )
        << "Pass NET::WMIconName to KWindowInfo" << endl;
    if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
        return QString::fromUtf8( d->info->iconName());
    if( !d->iconic_name_.isEmpty())
        return d->iconic_name_;
    return name();
}

bool KWindowInfo::isOnCurrentDesktop() const
{
    return isOnDesktop( KWindowSystem::currentDesktop());
}

bool KWindowInfo::isOnDesktop( int _desktop ) const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWindowInfo" << endl;
    if( KWindowSystem::mapViewport()) {
        if( onAllDesktops())
            return true;
        Window dummy;
        int x, y;
        unsigned int w, h, b, dp;
        XGetGeometry( QX11Info::display(), d->win_, &dummy, &x, &y, &w, &h, &b, &dp );
        // get global position
        XTranslateCoordinates( QX11Info::display(), d->win_, QX11Info::appRootWindow(), 0, 0, &x, &y, &dummy );
        return KWindowSystem::viewportWindowToDesktop( QRect( x, y, w, h )) == _desktop;
    }
    return d->info->desktop() == _desktop || d->info->desktop() == NET::OnAllDesktops;
}

bool KWindowInfo::onAllDesktops() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWindowInfo" << endl;
    if( KWindowSystem::mapViewport()) {
        if( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMState )
            return d->info->state() & NET::Sticky;
        NETWinInfo info( QX11Info::display(), d->win_, QX11Info::appRootWindow(), NET::WMState );
        return info.state() & NET::Sticky;
    }
    return d->info->desktop() == NET::OnAllDesktops;
}

int KWindowInfo::desktop() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWindowInfo" << endl;
    if( KWindowSystem::mapViewport()) {
        if( onAllDesktops())
            return NET::OnAllDesktops;
        Window r;
        int x, y;
        unsigned int w, h, b, dp;
        XGetGeometry( QX11Info::display(), d->win_, &r, &x, &y, &w, &h, &b, &dp );
        return KWindowSystem::viewportWindowToDesktop( QRect( x, y, w, h ));
    }
    return d->info->desktop();
}

QRect KWindowInfo::geometry() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMGeometry ) == 0, 176 )
        << "Pass NET::WMGeometry to KWindowInfo" << endl;
    return d->geometry_;
}

QRect KWindowInfo::frameGeometry() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMFrameExtents ) == 0, 176 )
        << "Pass NET::WMFrameExtents to KWindowInfo" << endl;
    return d->frame_geometry_;
}

WId KWindowInfo::transientFor() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2TransientFor ) == 0, 176 )
        << "Pass NET::WM2TransientFor to KWindowInfo" << endl;
    return d->info->transientFor();
}

WId KWindowInfo::groupLeader() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2GroupLeader ) == 0, 176 )
        << "Pass NET::WM2GroupLeader to KWindowInfo" << endl;
    return d->info->groupLeader();
}

QByteArray KWindowInfo::windowClassClass() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWindowInfo" << endl;
    return d->info->windowClassClass();
}

QByteArray KWindowInfo::windowClassName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWindowInfo" << endl;
    return d->info->windowClassName();
}

QByteArray KWindowInfo::windowRole() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowRole ) == 0, 176 )
        << "Pass NET::WM2WindowRole to KWindowInfo" << endl;
    return d->info->windowRole();
}

QByteArray KWindowInfo::clientMachine() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ClientMachine ) == 0, 176 )
        << "Pass NET::WM2ClientMachine to KWindowInfo" << endl;
    return d->info->clientMachine();
}

bool KWindowInfo::actionSupported( NET::Action action ) const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2AllowedActions ) == 0, 176 )
        << "Pass NET::WM2AllowedActions to KWindowInfo" << endl;
    if( KWindowSystem::allowedActionsSupported())
        return d->info->allowedActions() & action;
    else
        return true; // no idea if it's supported or not -> pretend it is
}

// see NETWM spec section 7.6
bool KWindowInfo::isMinimized() const
{
    if( mappingState() != NET::Iconic )
        return false;
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if(( state() & NET::Hidden ) != 0
	&& ( state() & NET::Shaded ) == 0 ) // shaded may have NET::Hidden too
        return true;
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return KWindowSystem::icccmCompliantMappingState() ? false : true;
}

