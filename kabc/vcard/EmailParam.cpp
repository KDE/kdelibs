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

#include <iostream.h>

#include <VCardEmailParam.h>
#include <VCardParam.h>
#include <VCardDefines.h>

using namespace VCARD;

EmailParam::EmailParam()
	:	Param()
{
	vDebug("ctor");
}

EmailParam::EmailParam(const EmailParam & x)
	:	Param(x),
		emailType_	(x.emailType_),
		pref_		(x.pref_)
{
}

EmailParam::EmailParam(const QCString & s)
	:	Param(s)
{
}

	EmailParam &
EmailParam::operator = (EmailParam & x)
{
	if (*this == x) return *this;
	
	emailType_	= x.emailType();
	pref_		= x.pref_;

	Param::operator = (x);
	return *this;
}

	EmailParam &
EmailParam::operator = (const QCString & s)
{
	Param::operator = (s);
	return *this;
}

	bool
EmailParam::operator == (EmailParam & x)
{
	parse();

	if (pref_)
		return (x.pref_ && x.emailType() == emailType_);

	return !x.pref();
}

EmailParam::~EmailParam()
{
}

	void
EmailParam::_parse()
{
#if 0
	Param::parseToList();
	
	SubParamListIterator it(subParamList_);
	
	pref_ = true;
	emailType_ = "";

	for (; it.current(); ++it) {
	
		if (qstricmp(it.current()->name(), "TYPE") == 0) {
			emailType_ = it.current()->value();
			continue;
		}
		
		if (qstricmp(it.current()->name(), "PREF") == 0) {
			pref_ = true;
		}
	}
#endif
}

	void
EmailParam::_assemble()
{
	strRep_ = "TYPE=";
	strRep_ += emailType_;

	if (pref_)
		strRep_ += ",PREF";
}

