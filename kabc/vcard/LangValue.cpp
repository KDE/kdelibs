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

