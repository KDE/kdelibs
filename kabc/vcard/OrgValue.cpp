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

#include <VCardRToken.h>

#include <VCardOrgValue.h>

#include <VCardValue.h>

using namespace VCARD;

OrgValue::OrgValue()
	:	Value()
{
}

OrgValue::OrgValue(const OrgValue & x)
	:	Value(x)
{
}

OrgValue::OrgValue(const QCString & s)
	:	Value(s)
{
}

	OrgValue &
OrgValue::operator = (OrgValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	OrgValue &
OrgValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
OrgValue::operator == (OrgValue & x)
{
	x.parse();
	return false;
}

OrgValue::~OrgValue()
{
}

	void
OrgValue::_parse()
{
	RTokenise(strRep_, ";", valueList_);
}

	void
OrgValue::_assemble()
{
	bool first(true);
	
	QStrListIterator it(valueList_);
	
	for (; it.current(); ++it) {
		if (!first) strRep_ += ';';
		strRep_ += it.current();
		first = false;
	}
}

	unsigned int
OrgValue::numValues()
{
	parse();
	return valueList_.count();
}

	QCString
OrgValue::value(unsigned int i)
{
	parse();
	return valueList_.at(i);
}

