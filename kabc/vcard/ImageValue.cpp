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

#include <VCardImageValue.h>

#include <VCardValue.h>

using namespace VCARD;

ImageValue::ImageValue()
	:	Value()
{
}

ImageValue::ImageValue(const ImageValue & x)
	:	Value(x)
{
}

ImageValue::ImageValue(const QCString & s)
	:	Value(s)
{
}

	ImageValue &
ImageValue::operator = (ImageValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	ImageValue &
ImageValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
ImageValue::operator == (ImageValue & x)
{
	x.parse();
	return false;
}

ImageValue::~ImageValue()
{
}

	void
ImageValue::_parse()
{
}

	void
ImageValue::_assemble()
{
}

