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

#include <VCardImgValue.h>

#include <VCardValue.h>

using namespace VCARD;

ImgValue::ImgValue()
	:	Value()
{
}

ImgValue::ImgValue(const ImgValue & x)
	:	Value(x)
{
}

ImgValue::ImgValue(const QCString & s)
	:	Value(s)
{
}

	ImgValue &
ImgValue::operator = (ImgValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	ImgValue &
ImgValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
ImgValue::operator == (ImgValue & x)
{
	x.parse();
	return false;
}

ImgValue::~ImgValue()
{
}

	void
ImgValue::_parse()
{
}

	void
ImgValue::_assemble()
{
}

