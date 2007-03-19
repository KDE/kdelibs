/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kserviceoffer.h"

class KServiceOffer::Private
{
public:
    Private()
        : iPreference( -1 ),
          bAllowAsDefault( false ),
          pService( 0 )
    {
    }

    int iPreference;
    bool bAllowAsDefault;
    KService::Ptr pService;
};

KServiceOffer::KServiceOffer()
    : d( new Private )
{
}

KServiceOffer::KServiceOffer( const KServiceOffer& _o )
    : d( new Private )
{
    d->pService = _o.d->pService;
    d->iPreference = _o.d->iPreference;
    d->bAllowAsDefault = _o.d->bAllowAsDefault;
}

KServiceOffer::KServiceOffer( const KService::Ptr& _service, int _pref, bool _default )
    : d( new Private )
{
    d->pService = _service;
    d->iPreference = _pref;
    d->bAllowAsDefault = _default;
}

KServiceOffer::KServiceOffer( const KService::Ptr& _service, int _pref )
    : d( new Private )
{
    d->pService = _service;
    d->iPreference = _pref;
    d->bAllowAsDefault = _service->allowAsDefault();
}

KServiceOffer::~KServiceOffer()
{
    delete d;
}

KServiceOffer& KServiceOffer::operator=( const KServiceOffer& rhs )
{
    if ( this == &rhs ) {
        return *this;
    }

    d->pService = rhs.d->pService;
    d->iPreference = rhs.d->iPreference;
    d->bAllowAsDefault = rhs.d->bAllowAsDefault;
    return *this;
}

bool KServiceOffer::operator< ( const KServiceOffer& _o ) const
{
  // Put offers allowed as default FIRST.
  if ( _o.d->bAllowAsDefault && !d->bAllowAsDefault )
    return false; // _o is default and not 'this'.
  if ( !_o.d->bAllowAsDefault && d->bAllowAsDefault )
    return true; // 'this' is default but not _o.
 // Both offers are allowed or not allowed as default
 // -> use preferences to sort them
 // The bigger the better, but we want the better FIRST
  return _o.d->iPreference < d->iPreference;
}

bool KServiceOffer::allowAsDefault() const
{
    return d->bAllowAsDefault;
}

int KServiceOffer::preference() const
{
    return d->iPreference;
}

void KServiceOffer::setPreference( int p )
{
    d->iPreference = p;
}

KService::Ptr KServiceOffer::service() const
{
    return d->pService;
}

bool KServiceOffer::isValid() const
{
    return d->iPreference >= 0;
}

