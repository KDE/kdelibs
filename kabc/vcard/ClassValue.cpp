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

#include <VCardClassValue.h>

#include <VCardValue.h>

using namespace VCARD;

ClassValue::ClassValue()
	:	Value()
{
}

ClassValue::ClassValue(const ClassValue & x)
	:	Value(x),
		classType_(x.classType_)
{
}

ClassValue::ClassValue(const QCString & s)
	:	Value(s)
{
}

	ClassValue &
ClassValue::operator = (ClassValue & x)
{
	if (*this == x) return *this;
	
	x.parse();
	
	classType_ = x.classType_;

	Value::operator = (x);
	return *this;
}

	ClassValue &
ClassValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
ClassValue::operator == (ClassValue & x)
{
	x.parse();
	return false;
}

ClassValue::~ClassValue()
{
}

	void
ClassValue::_parse()
{
	if (qstricmp(strRep_, "PUBLIC") == 0)
		classType_ = Public;
		
	else if (qstricmp(strRep_, "PRIVATE") == 0)
		classType_ = Private;
	
	else if (qstricmp(strRep_, "CONFIDENTIAL") == 0)
		classType_ = Confidential;
	
	else classType_ = Other;
}

	void
ClassValue::_assemble()
{
	switch (classType_) {
		
		case Public:
			strRep_ = "PUBLIC";
			break;
		
		case Private:
			strRep_ = "PRIVATE";
			break;
			
		case Confidential:
			strRep_ = "CONFIDENTIAL";
			break;
			
		default:
			break;
	}
}

