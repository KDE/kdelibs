/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
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

#include <qstrlist.h>

#include <VCardRToken.h>
#include <VCardNValue.h>
#include <VCardValue.h>
#include <VCardDefines.h>

using namespace VCARD;

NValue::NValue()
	:	Value()
{
	vDebug("ctor");
}

NValue::NValue(const NValue & x)
	:	Value(x),
		family_	(x.family_),
		given_	(x.given_),
		middle_	(x.middle_),
		prefix_	(x.prefix_),
		suffix_	(x.suffix_)
{
}

NValue::NValue(const QCString & s)
	:	Value(s)
{
	vDebug("ctor");
}

	NValue &
NValue::operator = (NValue & x)
{
	if (*this == x) return *this;
	
	family_	= x.family_;
	given_	= x.given_;
	middle_	= x.middle_;
	prefix_	= x.prefix_;
	suffix_	= x.suffix_;

	Value::operator = (x);
	return *this;
}

	NValue &
NValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
NValue::operator == (NValue & x)
{
	x.parse();

	return (
		family_	== x.family_	&&
		given_	== x.given_	&&
		middle_	== x.middle_	&&
		prefix_	== x.prefix_	&&
		suffix_ == x.suffix_);
}

NValue::~NValue()
{
}

	NValue *
NValue::clone()
{
	return new NValue( *this );
}

	void
NValue::_parse()
{
	QStrList l;
	RTokenise(strRep_, ";", l);
	
	for (unsigned int i = 0; i < l.count(); i++) {

		switch (i) {
			case 0: family_	= l.at(0);	break;
			case 1: given_	= l.at(1);	break;
			case 2: middle_	= l.at(2);	break;
			case 3: prefix_	= l.at(3);	break;
			case 4: suffix_	= l.at(4);	break;
			default:			break;
		}
	}
}

	void
NValue::_assemble()
{
	strRep_ =		family_;
	strRep_ += ";" +	given_;
	strRep_ += ";" +	middle_;
	strRep_ += ";" +	prefix_;
	strRep_ += ";" +	suffix_;
}

