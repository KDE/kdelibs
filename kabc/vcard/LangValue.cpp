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

#include <VCardLangValue.h>

#include <VCardValue.h>

using namespace VCARD;

LangValue::LangValue()
	:	Value()
{
}

LangValue::LangValue(const LangValue & x)
	:	Value(x)
{
}

LangValue::LangValue(const QCString & s)
	:	Value(s)
{
}

	LangValue &
LangValue::operator = (LangValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	LangValue &
LangValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
LangValue::operator == (LangValue & x)
{
	x.parse();
	return false;
}

LangValue::~LangValue()
{
}

	void
LangValue::_parse()
{
	QStrList l;
	RTokenise(strRep_, "-", l);
	
	if (l.count() == 0) return;
	
	primary_ = l.at(0);
	
	l.remove(0u);
	
	subtags_ = l;
}

	void
LangValue::_assemble()
{
	strRep_ = primary_;
	
	QStrListIterator it(subtags_);
	
	for (; it.current(); ++it)
		strRep_ += QCString('-') + it.current();
}

	QCString
LangValue::primary()
{
	parse();
	return primary_;
}
	
	QStrList
LangValue::subtags()
{
	parse();
	return subtags_;
}
	
	void
LangValue::setPrimary(const QCString & s)
{
	parse();
	primary_ = s;
}

	void
LangValue::setSubTags(const QStrList & l)
{
	parse();
	subtags_ = l;
}

