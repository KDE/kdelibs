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

#include <QDebug>
#include <kxerrorhandler_p.h>
#include <netwm.h>
#include <QBitmap>
#include <QDesktopWidget>
#include <QDialog>
#include <QX11Info>
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
    if( ( properties & NET::WMDesktop ) && KWindowSystem::mapViewport() )
	properties |= NET::WMGeometry; // for viewports, the desktop (workspace) is determined from the geometry
    properties |= NET::XAWMState; // force to get error detection for valid()
    unsigned long props[ 2 ] = { properties, properties2 };
    d->info = new NETWinInfo( QX11Info::connection(), _win, QX11Info::appRootWindow(), props, 2 );
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
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMState))
        qWarning() << "Pass NET::WMState to KWindowInfo";
#endif
    return d->info->state();
}

bool KWindowInfo::hasState( unsigned long s ) const
{
    return ( state() & s ) == s;
}

NET::MappingState KWindowInfo::mappingState() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::XAWMState))
        qWarning() << "Pass NET::XAWMState to KWindowInfo";
#endif
    return d->info->mappingState();
}

NETExtendedStrut KWindowInfo::extendedStrut() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ExtendedStrut))
        qWarning() << "Pass NET::WM2ExtendedStrut to KWindowInfo";
#endif
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
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMWindowType))
        qWarning() << "Pass NET::WMWindowType to KWindowInfo";
#endif
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
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleName))
        qWarning() << "Pass NET::WMVisibleName to KWindowInfo";
#endif
    return d->info->visibleName() && d->info->visibleName()[ 0 ] != '\0'
        ? QString::fromUtf8(d->info->visibleName()) : name();
}

QString KWindowInfo::name() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMName))
        qWarning() << "Pass NET::WMName to KWindowInfo";
#endif
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
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleIconName))
        qWarning() << "Pass NET::WMVisibleIconName to KWindowInfo";
#endif
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
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMIconName))
        qWarning() << "Pass NET::WMIconName to KWindowInfo";
#endif
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
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop))
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
#endif
    if( KWindowSystem::mapViewport()) {
        if( onAllDesktops())
            return true;
        return KWindowSystem::viewportWindowToDesktop( d->geometry_ ) == _desktop;
    }
    return d->info->desktop() == _desktop || d->info->desktop() == NET::OnAllDesktops;
}

bool KWindowInfo::onAllDesktops() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop))
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
#endif
    if( KWindowSystem::mapViewport()) {
        if( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMState )
            return d->info->state() & NET::Sticky;
        NETWinInfo info( QX11Info::connection(), d->win_, QX11Info::appRootWindow(), NET::WMState );
        return info.state() & NET::Sticky;
    }
    return d->info->desktop() == NET::OnAllDesktops;
}

int KWindowInfo::desktop() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop))
        qWarning() << "Pass NET::WMDesktop to KWindowInfo";
#endif
    if( KWindowSystem::mapViewport()) {
        if( onAllDesktops())
            return NET::OnAllDesktops;
        return KWindowSystem::viewportWindowToDesktop( d->geometry_ );
    }
    return d->info->desktop();
}

QRect KWindowInfo::geometry() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMGeometry))
        qWarning() << "Pass NET::WMGeometry to KWindowInfo";
#endif
    return d->geometry_;
}

QRect KWindowInfo::frameGeometry() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMFrameExtents))
        qWarning() << "Pass NET::WMFrameExtents to KWindowInfo";
#endif
    return d->frame_geometry_;
}

WId KWindowInfo::transientFor() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2TransientFor))
        qWarning() << "Pass NET::WM2TransientFor to KWindowInfo";
#endif
    return d->info->transientFor();
}

WId KWindowInfo::groupLeader() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2GroupLeader))
        qWarning() << "Pass NET::WM2GroupLeader to KWindowInfo";
#endif
    return d->info->groupLeader();
}

QByteArray KWindowInfo::windowClassClass() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass))
        qWarning() << "Pass NET::WM2WindowClass to KWindowInfo";
#endif
    return d->info->windowClassClass();
}

QByteArray KWindowInfo::windowClassName() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass))
        qWarning() << "Pass NET::WM2WindowClass to KWindowInfo";
#endif
    return d->info->windowClassName();
}

QByteArray KWindowInfo::windowRole() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowRole))
        qWarning() << "Pass NET::WM2WindowRole to KWindowInfo";
#endif
    return d->info->windowRole();
}

QByteArray KWindowInfo::clientMachine() const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ClientMachine))
        qWarning() << "Pass NET::WM2ClientMachine to KWindowInfo";
#endif
    return d->info->clientMachine();
}

bool KWindowInfo::actionSupported( NET::Action action ) const
{
#if !defined(KDE_NO_WARNING_OUTPUT)
    if (!(d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2AllowedActions))
        qWarning() << "Pass NET::WM2AllowedActions to KWindowInfo";
#endif
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

