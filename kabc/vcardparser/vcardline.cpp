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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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

void VCardLine::setGroup( const QString& group )
{
  mGroup = group;
}

QString VCardLine::group() const
{
  return mGroup;
}

bool VCardLine::hasGroup() const
{
  return !mGroup.isEmpty();
}

QStringList VCardLine::parameterList() const
{
  return mParamMap.keys();
}

void VCardLine::addParameter( const QString& param, const QString& value )
{
  QStringList &list = mParamMap[ param ];
  if ( !list.contains( value ) ) // not included yet
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
    return QString();
  else {
    if ( (*it).isEmpty() )
      return QString();
    else
      return (*it).first();
  }
}
