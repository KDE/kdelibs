/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include "vcardline.h"

class VCardLine::VCardLinePrivate
{
  public:
    QMap< QString, QStringList > mParamMap;
    QString mIdentifier;
    QVariant mValue;
};

VCardLine::VCardLine()
  : d( new VCardLinePrivate )
{
}

VCardLine::VCardLine( const QString &identifier )
{
  d->mIdentifier = identifier.lower();
}

VCardLine::VCardLine( const QString &identifier, const QVariant &value )
{
  d->mIdentifier = identifier.lower();
  d->mValue = value;
}

VCardLine::~VCardLine()
{
  delete d;
  d = 0;
}

void VCardLine::setIdentifier( const QString& identifier )
{
  d->mIdentifier = identifier.lower();
}

QString VCardLine::identifier() const
{
  return d->mIdentifier;
}

void VCardLine::setValue( const QVariant& value )
{
  d->mValue = value;
}

QVariant VCardLine::value() const
{
  return d->mValue;
}

QStringList VCardLine::parameterList() const
{
  return d->mParamMap.keys();
}

void VCardLine::addParameter( const QString& param, const QString& value )
{
  QValueList<QString> &list = d->mParamMap[ param ];

  if ( list.find( value ) == list.end() ) // not included yet
    list.append( value );
}

QStringList VCardLine::parameters( const QString& param ) const
{
  return d->mParamMap[ param ];
}

QString VCardLine::parameter( const QString& param ) const
{
  return d->mParamMap[ param ][ 0 ];
}
