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

#include <VCardTextBinValue.h>

#include <VCardValue.h>

using namespace VCARD;

TextBinValue::TextBinValue()
	:	Value()
{
}

TextBinValue::TextBinValue(const TextBinValue & x)
	:	Value(x)
{
}

TextBinValue::TextBinValue(const QCString & s)
	:	Value(s)
{
}

	TextBinValue &
TextBinValue::operator = (TextBinValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	TextBinValue &
TextBinValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
TextBinValue::operator == (TextBinValue & x)
{
	x.parse();
	return false;
}

TextBinValue::~TextBinValue()
{
}

	void
TextBinValue::_parse()
{
}

	void
TextBinValue::_assemble()
{
}

