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
#include <VCardAdrValue.h>
#include <VCardValue.h>
#include <VCardDefines.h>

using namespace VCARD;

AdrValue::AdrValue()
	:	Value()
{
}

AdrValue::AdrValue(const AdrValue & x)
	:	Value(x),
		poBox_		(x.poBox_),
		extAddress_	(x.extAddress_),
		street_		(x.street_),
		locality_	(x.locality_),
		region_		(x.region_),
		postCode_	(x.postCode_),
		countryName_	(x.countryName_)
{
}

AdrValue::AdrValue(const QCString & s)
	:	Value(s)
{
}

	AdrValue &
AdrValue::operator = (AdrValue & x)
{
	if (*this == x) return *this;

	poBox_		= x.poBox_;
	extAddress_	= x.extAddress_;
	street_		= x.street_;
	locality_	= x.locality_;
	region_		= x.region_;
	postCode_	= x.postCode_;
	countryName_	= x.countryName_;

	Value::operator = (x);
	return *this;
}

	AdrValue &
AdrValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
AdrValue::operator == (AdrValue & x)
{
	parse();
	x.parse();
	
	return (
		poBox_		== x.poBox_		&&
		extAddress_	== x.extAddress_	&&
		street_		== x.street_		&&
		locality_	== x.locality_		&&
		region_		== x.region_		&&
		postCode_	== x.postCode_		&&
		countryName_	== x.countryName_);
}

AdrValue::~AdrValue()
{
}

	AdrValue *
AdrValue::clone()
{
	return new AdrValue( *this );
}

	void
AdrValue::_parse()
{
	QStrList l;
	RTokenise(strRep_, ";", l);
	
	for (unsigned int i = 0; i < l.count(); i++) {
		
		switch (i) {
			
			case 0: poBox_		= l.at(0);	break;
			case 1: extAddress_	= l.at(1);	break;
			case 2: street_		= l.at(2);	break;
			case 3: locality_	= l.at(3);	break;
			case 4: region_		= l.at(4);	break;
			case 5: postCode_	= l.at(5);	break;
			case 6: countryName_	= l.at(6);	break;
			default:							break;
		}
	}
}

	void
AdrValue::_assemble()
{
	vDebug("AdrValue::assemble_");

	strRep_ =		poBox_;
	strRep_ += ";" +	extAddress_;
	strRep_ += ";" +	street_;
	strRep_ += ";" +	locality_;
	strRep_ += ";" +	region_;
	strRep_ += ";" +	postCode_;
	strRep_ += ";" +	countryName_;
}

