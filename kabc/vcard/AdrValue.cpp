/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

