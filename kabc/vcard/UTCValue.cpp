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

#include <VCardUTCValue.h>

#include <VCardValue.h>

using namespace VCARD;

UTCValue::UTCValue()
	:	Value()
{
}

UTCValue::UTCValue(const UTCValue & x)
	:	Value(x)
{
}

UTCValue::UTCValue(const QCString & s)
	:	Value(s)
{
}

	UTCValue &
UTCValue::operator = (UTCValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	UTCValue &
UTCValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
UTCValue::operator == (UTCValue & x)
{
	x.parse();
	return false;
}

UTCValue::~UTCValue()
{
}

	void
UTCValue::_parse()
{
	positive_ = (strRep_[0] == '+');
	
	int colon = strRep_.find(':');
	
	if (colon == -1) // Not valid.
		return;

	hour_	= strRep_.mid(1, colon).toInt();
	minute_	= strRep_.right(2).toInt();
}

	void
UTCValue::_assemble()
{
	strRep_ = (positive_ ? '+' : '-');
	
	if (hour_ < 10) // Pointless zero padding.
		strRep_ += '0';
	
	strRep_ += QCString().setNum(hour_) + ':';
	
	if (minute_ < 10) // Pointless zero padding.
		strRep_ += '0'; 
	
	strRep_ += QCString().setNum(minute_);
}

