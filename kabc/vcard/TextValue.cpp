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

#include <VCardTextValue.h>

#include <VCardValue.h>

using namespace VCARD;

TextValue::TextValue()
	:	Value()
{
}

TextValue::TextValue(const TextValue & x)
	:	Value(x)
{
}

TextValue::TextValue(const QCString & s)
	:	Value(s)
{
}

	TextValue &
TextValue::operator = (TextValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	TextValue &
TextValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
TextValue::operator == (TextValue & x)
{
	return strRep_ = x.strRep_;
}

TextValue::~TextValue()
{
}

  TextValue *
TextValue::clone()
{
  return new TextValue( *this );
}

	void
TextValue::_parse()
{
}

	void
TextValue::_assemble()
{
}

