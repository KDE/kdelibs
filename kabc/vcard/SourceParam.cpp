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

#include <VCardSourceParam.h>

#include <VCardParam.h>

using namespace VCARD;

SourceParam::SourceParam()
	:	Param(),
		type_(SourceParam::TypeUnknown)
{
}

SourceParam::SourceParam(const SourceParam & x)
	:	Param(x),
		type_	(x.type_),
		par_	(x.par_),
		val_	(x.val_)
{
}

SourceParam::SourceParam(const QCString & s)
	:	Param(s),
		type_(SourceParam::TypeUnknown)
{
}

	SourceParam &
SourceParam::operator = (SourceParam & x)
{
	if (*this == x) return *this;
	type_	= x.type();
	par_	= x.par();
	val_	= x.val();

	Param::operator = (x);
	return *this;
}

	SourceParam &
SourceParam::operator = (const QCString & s)
{
	Param::operator = (s);
	return *this;
}

	bool
SourceParam::operator == (SourceParam & x)
{
	x.parse();
	return false;
}

SourceParam::~SourceParam()
{
}

	void
SourceParam::_parse()
{
	int i = strRep_.find('=');
	if (i == -1) // Invalid
		return;
	
	par_ = strRep_.left(i);
	val_ = strRep_.right(strRep_.length() - i - 1);
	
	if (qstricmp(par_, "VALUE") == 0 && qstricmp(val_, "uri") == 0)
		type_ = TypeValue;
	else if (qstricmp(par_, "CONTEXT") == 0 && qstricmp(val_, "word") == 0)
		type_ = TypeContext;
	else if (qstrnicmp(par_, "X-", 2) == 0) {
		type_ = TypeX;
	}
	else type_ = TypeUnknown;
	
}

	void
SourceParam::_assemble()
{
	if (type_ == TypeValue)
		strRep_ = "VALUE=uri";
	else if (type_ == TypeContext)
		strRep_ = "CONTEXT=word";
	else if (type_ == TypeX)
		strRep_ = par_ + "=" + val_;
	else strRep_ = "";
}

