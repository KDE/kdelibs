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

#include <VCardTextBinParam.h>

#include <VCardParam.h>

using namespace VCARD;

TextBinParam::TextBinParam()
	:	Param()
{
}

TextBinParam::TextBinParam(const TextBinParam & x)
	:	Param(x)
{
}

TextBinParam::TextBinParam(const QCString & s)
	:	Param(s)
{
}

	TextBinParam &
TextBinParam::operator = (TextBinParam & x)
{
	if (*this == x) return *this;

	Param::operator = (x);
	return *this;
}

	TextBinParam &
TextBinParam::operator = (const QCString & s)
{
	Param::operator = (s);
	return *this;
}

	bool
TextBinParam::operator == (TextBinParam & x)
{
	x.parse();
	return false;
}

TextBinParam::~TextBinParam()
{
}

	void
TextBinParam::_parse()
{
}

	void
TextBinParam::_assemble()
{
}

