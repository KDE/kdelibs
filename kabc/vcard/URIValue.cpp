/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <VCardURIValue.h>

#include <VCardValue.h>

using namespace VCARD;

URIValue::URIValue()
	:	Value()
{
}

URIValue::URIValue(const QCString & scheme, const QCString & schemeSpecificPart)
	:	Value(),
		scheme_				(scheme),
		schemeSpecificPart_	(schemeSpecificPart)
{
	parsed_ = true;
}

URIValue::URIValue(const URIValue & x)
	:	Value				(x),
		scheme_				(x.scheme_),
		schemeSpecificPart_	(x.schemeSpecificPart_)
{
}

URIValue::URIValue(const QCString & s)
	:	Value(s)
{
}

	URIValue &
URIValue::operator = (URIValue & x)
{
	if (*this == x) return *this;
	
	scheme_				= x.scheme_;
	schemeSpecificPart_	= x.schemeSpecificPart_;

	Value::operator = (x);
	return *this;
}

	URIValue &
URIValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
URIValue::operator == (URIValue & x)
{
	x.parse();
	return (
		(scheme_				== x.scheme_) &&
		(schemeSpecificPart_	== x.schemeSpecificPart_));

	return false;
}

URIValue::~URIValue()
{
}

	void
URIValue::_parse()
{
	int split = strRep_.find(':');
	if (split == -1)
		return;
	
	scheme_ = strRep_.left(split);
	schemeSpecificPart_ = strRep_.mid(split + 1);
}

	void
URIValue::_assemble()
{
	strRep_ = scheme_ + ':' + schemeSpecificPart_;
}

	QCString
URIValue::scheme()
{
	parse();
	return scheme_;
}

	QCString
URIValue::schemeSpecificPart()
{
	parse();
	return schemeSpecificPart_;
}

	void
URIValue::setScheme(const QCString & s)
{
	parse();
	scheme_ = s;
}

	void
URIValue::setSchemeSpecificPart(const QCString & s)
{
	parse();
	schemeSpecificPart_ = s;
}

