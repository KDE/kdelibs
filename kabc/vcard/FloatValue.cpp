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

#include <VCardFloatValue.h>

#include <VCardValue.h>

using namespace VCARD;

FloatValue::FloatValue()
	:	Value()
{
}

FloatValue::FloatValue(float f)
	:	Value	(),
		value_	(f)
{
	parsed_ = true;
}

FloatValue::FloatValue(const FloatValue & x)
	:	Value(x)
{
	value_ = x.value_;
}

FloatValue::FloatValue(const QCString & s)
	:	Value(s)
{
}

	FloatValue &
FloatValue::operator = (FloatValue & x)
{
	if (*this == x) return *this;
	
	x.parse();
	value_ = x.value_;

	Value::operator = (x);
	return *this;
}

	FloatValue &
FloatValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
FloatValue::operator == (FloatValue & x)
{
	x.parse();
	return (value_ == x.value_);
}

FloatValue::~FloatValue()
{
}

	void
FloatValue::_parse()
{
	bool negative(false);
	
	if (strRep_[0] == '-' || strRep_[1] == '+') {
		
		if (strRep_[0] == '-')
			negative = true;
		
		strRep_.remove(0, 1);
	}
	
	value_ = strRep_.toFloat();
	if (negative)
		value_ = -value_;
}

	void
FloatValue::_assemble()
{
	strRep_ = QCString().setNum(value_);
}

	float
FloatValue::value()
{
	parse();
	return value_;
}

	void
FloatValue::setValue(float f)
{
	parsed_ = true;
	value_ = f;
}

