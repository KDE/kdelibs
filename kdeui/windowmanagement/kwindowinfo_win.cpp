/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Carlo Segato (brandon.ml@gmail.com)

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

#include <QRect>

KWindowInfo::KWindowInfo( WId win, unsigned long, unsigned long )
{
}

// this one is only to make QList<> or similar happy
KWindowInfo::KWindowInfo()
{
}

KWindowInfo::~KWindowInfo()
{
}

KWindowInfo::KWindowInfo( const KWindowInfo& wininfo )
{
}

KWindowInfo& KWindowInfo::operator=( const KWindowInfo& wininfo )
{
}

bool KWindowInfo::valid( bool withdrawn_is_valid ) const
{
}

WId KWindowInfo::win() const
{
}

unsigned long KWindowInfo::state() const
{
    return 0;
}

bool KWindowInfo::hasState( unsigned long s ) const
{
    return false;
}

bool KWindowInfo::isMinimized() const
{
}

NET::MappingState KWindowInfo::mappingState() const
{
}

NETExtendedStrut KWindowInfo::extendedStrut() const
{
}

NET::WindowType KWindowInfo::windowType( int supported_types ) const
{
}

QString KWindowInfo::visibleNameWithState() const
{
}

QString KWindowInfo::visibleName() const
{
}

QString KWindowInfo::name() const
{
}

QString KWindowInfo::visibleIconNameWithState() const
{
}

QString KWindowInfo::visibleIconName() const
{
}

QString KWindowInfo::iconName() const
{
}

bool KWindowInfo::isOnCurrentDesktop() const
{
}

bool KWindowInfo::isOnDesktop( int _desktop ) const
{
}

bool KWindowInfo::onAllDesktops() const
{
}

int KWindowInfo::desktop() const
{
}

QRect KWindowInfo::geometry() const
{
}

QRect KWindowInfo::frameGeometry() const
{
}

bool KWindowInfo::actionSupported( NET::Action action ) const
{
    return true; // no idea if it's supported or not -> pretend it is
}

#if 0
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
#endif
