/*
    This file is part of libvcard.
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

    $Id$
*/

#include <VCardGeoValue.h>

#include <VCardValue.h>

#include <kdebug.h>

using namespace VCARD;

GeoValue::GeoValue()
	:	Value()
{
}

GeoValue::GeoValue(const GeoValue & x)
	:	Value(x)
{
}

GeoValue::GeoValue(const QCString & s)
	:	Value(s)
{
}

	GeoValue &
GeoValue::operator = (GeoValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	GeoValue &
GeoValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
GeoValue::operator == (GeoValue & x)
{
	x.parse();
	return false;
}

GeoValue::~GeoValue()
{
}

	void
GeoValue::_parse()
{
    int semiColon = strRep_.find( ";" );

    if ( semiColon == -1 ) // invalid
	return;

    latitude_	= strRep_.left( semiColon ).toFloat();
    longitude_	= strRep_.mid( semiColon + 1, strRep_.length() - semiColon ).toFloat();
}

	void
GeoValue::_assemble()
{
    strRep_.sprintf( "%.6f;%.6f", latitude_, longitude_ );
}

	void
GeoValue::setLatitude( float value )
{
    latitude_ = value;
}

	void
GeoValue::setLongitude( float value )
{
    longitude_ = value;
}

	float
GeoValue::latitude()
{
    return latitude_;
}

	float
GeoValue::longitude()
{
    return longitude_;
}
