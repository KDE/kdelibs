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
#include <VCardAdrParam.h>
#include <VCardParam.h>

using namespace VCARD;

AdrParam::AdrParam()
	:	Param()
{
}

AdrParam::AdrParam(const AdrParam & x)
	:	Param(x),
		adrTypeList_	(x.adrTypeList_)
{
}

AdrParam::AdrParam(const QCString & s)
	:	Param(s)
{
}

	AdrParam &
AdrParam::operator = (AdrParam & x)
{
	if (*this == x) return *this;
	
	adrTypeList_	= x.adrTypeList();
	textParam_		= x.textParam();

	Param::operator = (x);
	return *this;
}

	AdrParam &
AdrParam::operator = (const QCString & s)
{
	Param::operator = (s);
	
	adrTypeList_.clear();
	textParam_.truncate(0);
	
	return *this;
}

	bool
AdrParam::operator == (AdrParam & x)
{
	parse();

	if (!x.textParam().isEmpty())
		return (x.textParam_ == textParam_);

	if (x.adrTypeList().count() != adrTypeList_.count())
		return false;
	
	QStrListIterator it(x.adrTypeList_);
	
	for (; it.current(); ++it)
		if (!adrTypeList_.find(it.current()))
			return false;
	
	return true;
}

AdrParam::~AdrParam()
{
}

	void
AdrParam::_parse()
{
	adrTypeList_.clear();

	if (strRep_.left(4) != "TYPE") {
		textParam_ = strRep_;
		return;
	}
	
	if (!strRep_.contains('='))
		return;
	
	RTokenise(strRep_, ",", adrTypeList_);
}

	void
AdrParam::_assemble()
{
	if (!textParam_.isEmpty()) {
		strRep_ = textParam_;
		return;
	}
	
	QStrListIterator it(adrTypeList_);
	
	for (; it.current(); ++it) {
		
		strRep_ += it.current();
		
		if (it.current() != adrTypeList_.last())
			strRep_ += ',';
	}
}
