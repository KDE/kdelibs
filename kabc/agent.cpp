/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include "addressee.h"

#include "agent.h"

using namespace KABC;

Agent::Agent()
  : mAddressee( 0 ), mIntern( true )
{
}

Agent::Agent( const QString &url )
  : mAddressee( 0 ),mUrl( url ), mIntern( false )
{
}

Agent::Agent( Addressee *addressee )
  : mAddressee( addressee ), mIntern( true )
{
}

Agent::~Agent()
{
}

bool Agent::operator==( const Agent &a ) const
{
  if ( mIntern != a.mIntern )
    return false;

  if ( !mIntern ) {
    if ( mUrl != a.mUrl )
      return false;
  } else {
    if ( mAddressee && !a.mAddressee ) return false;
    if ( !mAddressee && a.mAddressee ) return false;
    if ( !mAddressee && !a.mAddressee ) return false;
    if ( (*mAddressee) != (*a.mAddressee) ) return false;
  }

  return true;
}

bool Agent::operator!=( const Agent &a ) const
{
  return !( a == *this );
}

void Agent::setUrl( const QString &url )
{
  mUrl = url;
  mIntern = false;
}

void Agent::setAddressee( Addressee *addressee )
{
  mAddressee = addressee;
  mIntern = true;
}

bool Agent::isIntern() const
{
  return mIntern;
}

QString Agent::url() const
{
  return mUrl;
}

Addressee *Agent::addressee() const
{
  return mAddressee;
}

QString Agent::asString() const
{
  if ( mIntern )
    return "intern agent";
  else
    return mUrl;
}

QDataStream &KABC::operator<<( QDataStream &s, const Agent &agent )
{
  Q_UINT32 hasAddressee = ( agent.mAddressee != 0 );

  s << agent.mIntern << agent.mUrl << hasAddressee;
  if ( hasAddressee )
    s << (*agent.mAddressee);

  return s;
}

QDataStream &KABC::operator>>( QDataStream &s, Agent &agent )
{
  Q_UINT32 hasAddressee;

  s >> agent.mIntern >> agent.mUrl >> hasAddressee;

  if ( hasAddressee ) {
    agent.mAddressee = new Addressee;
    s >> (*agent.mAddressee);
  }

  return s;
}
