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

#include <VCardTelParam.h>

#include <VCardParam.h>

using namespace VCARD;

TelParam::TelParam()
	:	Param()
{
}

TelParam::TelParam(const TelParam & x)
	:	Param(x)
{
}

TelParam::TelParam(const QCString & s)
	:	Param(s)
{
}

	TelParam &
TelParam::operator = (TelParam & x)
{
	if (*this == x) return *this;

	Param::operator = (x);
	return *this;
}

	TelParam &
TelParam::operator = (const QCString & s)
{
	Param::operator = (s);
	return *this;
}

	bool
TelParam::operator == (TelParam & x)
{
	x.parse();
	return false;
}

TelParam::~TelParam()
{
}

	void
TelParam::_parse()
{
}

	void
TelParam::_assemble()
{
}

