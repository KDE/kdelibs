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

using namespace KABC;

VCardLine::VCardLine()
{
}

VCardLine::VCardLine( const QString &identifier )
{
  mIdentifier = identifier;
}

VCardLine::VCardLine( const QString &identifier, const QVariant &value )
{
  mIdentifier = identifier;
  mValue = value;
}

VCardLine::VCardLine( const VCardLine& line )
{
  mParamMap = line.mParamMap;
  mValue = line.mValue;
  mIdentifier = line.mIdentifier;
}

VCardLine::~VCardLine()
{
}

VCardLine& VCardLine::operator=( const VCardLine& line )
{
  if ( &line == this )
    return *this;

  mParamMap = line.mParamMap;
  mValue = line.mValue;
  mIdentifier = line.mIdentifier;

  return *this;
}

void VCardLine::setIdentifier( const QString& identifier )
{
  mIdentifier = identifier;
}

QString VCardLine::identifier() const
{
  return mIdentifier;
}

void VCardLine::setValue( const QVariant& value )
{
  mValue = value;
}

QVariant VCardLine::value() const
{
  return mValue;
}

QStringList VCardLine::parameterList() const
{
  return mParamMap.keys();
}

void VCardLine::addParameter( const QString& param, const QString& value )
{
  QStringList &list = mParamMap[ param ];
  if ( list.findIndex( value ) == -1 ) // not included yet
    list.append( value );
}

QStringList VCardLine::parameters( const QString& param ) const
{
  ParamMap::ConstIterator it = mParamMap.find( param );
  if ( it == mParamMap.end() )
    return QStringList();
  else
    return *it;
}

QString VCardLine::parameter( const QString& param ) const
{
  ParamMap::ConstIterator it = mParamMap.find( param );
  if ( it == mParamMap.end() )
    return QString::null;
  else {
    if ( (*it).isEmpty() )
      return QString::null;
    else
      return (*it).first();
  }
}
