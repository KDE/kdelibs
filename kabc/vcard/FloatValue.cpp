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

#include <VCardFloatValue.h>

#include <VCardValue.h>

using namespace VCARD;

FloatValue::FloatValue()
	:	Value()
{
}

FloatValue::FloatValue(float f)
	:	Value	(),
		value_	(f)
{
	parsed_ = true;
}

FloatValue::FloatValue(const FloatValue & x)
	:	Value(x)
{
	value_ = x.value_;
}

FloatValue::FloatValue(const QCString & s)
	:	Value(s)
{
}

	FloatValue &
FloatValue::operator = (FloatValue & x)
{
	if (*this == x) return *this;
	
	x.parse();
	value_ = x.value_;

	Value::operator = (x);
	return *this;
}

	FloatValue &
FloatValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
FloatValue::operator == (FloatValue & x)
{
	x.parse();
	return (value_ == x.value_);
}

FloatValue::~FloatValue()
{
}

	void
FloatValue::_parse()
{
	bool negative(false);
	
	if (strRep_[0] == '-' || strRep_[1] == '+') {
		
		if (strRep_[0] == '-')
			negative = true;
		
		strRep_.remove(0, 1);
	}
	
	value_ = strRep_.toFloat();
	if (negative)
		value_ = -value_;
}

	void
FloatValue::_assemble()
{
	strRep_ = QCString().setNum(value_);
}

	float
FloatValue::value()
{
	parse();
	return value_;
}

	void
FloatValue::setValue(float f)
{
	parsed_ = true;
	value_ = f;
}

