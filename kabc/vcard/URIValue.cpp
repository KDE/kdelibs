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

#include <VCardURIValue.h>

#include <VCardValue.h>

using namespace VCARD;

URIValue::URIValue()
	:	Value()
{
}

URIValue::URIValue(const QCString & scheme, const QCString & schemeSpecificPart)
	:	Value(),
		scheme_				(scheme),
		schemeSpecificPart_	(schemeSpecificPart)
{
	parsed_ = true;
}

URIValue::URIValue(const URIValue & x)
	:	Value				(x),
		scheme_				(x.scheme_),
		schemeSpecificPart_	(x.schemeSpecificPart_)
{
}

URIValue::URIValue(const QCString & s)
	:	Value(s)
{
}

	URIValue &
URIValue::operator = (URIValue & x)
{
	if (*this == x) return *this;
	
	scheme_				= x.scheme_;
	schemeSpecificPart_	= x.schemeSpecificPart_;

	Value::operator = (x);
	return *this;
}

	URIValue &
URIValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
URIValue::operator == (URIValue & x)
{
	x.parse();
	return (
		(scheme_				== x.scheme_) &&
		(schemeSpecificPart_	== x.schemeSpecificPart_));

	return false;
}

URIValue::~URIValue()
{
}

	void
URIValue::_parse()
{
	int split = strRep_.find(':');
	if (split == -1)
		return;
	
	scheme_ = strRep_.left(split);
	schemeSpecificPart_ = strRep_.mid(split + 1);
}

	void
URIValue::_assemble()
{
	strRep_ = scheme_ + ':' + schemeSpecificPart_;
}

	QCString
URIValue::scheme()
{
	parse();
	return scheme_;
}

	QCString
URIValue::schemeSpecificPart()
{
	parse();
	return schemeSpecificPart_;
}

	void
URIValue::setScheme(const QCString & s)
{
	parse();
	scheme_ = s;
}

	void
URIValue::setSchemeSpecificPart(const QCString & s)
{
	parse();
	schemeSpecificPart_ = s;
}

