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

#include <VCardUTCValue.h>

#include <VCardValue.h>

using namespace VCARD;

UTCValue::UTCValue()
	:	Value()
{
}

UTCValue::UTCValue(const UTCValue & x)
	:	Value(x),	positive_(x.positive_), hour_(x.hour_), minute_(x.minute_)

{
}

UTCValue::UTCValue(const QCString & s)
	:	Value(s)
{
}

	UTCValue &
UTCValue::operator = (UTCValue & x)
{
	if (*this == x) return *this;

  positive_ = x.positive_;
  hour_ = x.hour_;
  minute_ = x.minute_;

	Value::operator = (x);
	return *this;
}

	UTCValue &
UTCValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
UTCValue::operator == (UTCValue & x)
{
	x.parse();

  if (positive_ != x.positive_) return false;
  if (hour_ != x.hour_) return false;
  if (minute_ != x.minute_) return false;

	return true;
}

UTCValue::~UTCValue()
{
}

	void
UTCValue::_parse()
{
  if ( strRep_.isEmpty() )
    return;

	positive_ = (strRep_[0] == '+');
	
	int colon = strRep_.find(':');
	
	if (colon == -1) // Not valid.
		return;

	hour_	= strRep_.mid(1, colon).toInt();
	minute_	= strRep_.right(2).toInt();
}

	void
UTCValue::_assemble()
{
  strRep_.sprintf( "%c%.2i:%.2i", (positive_ ? '+' : '-'), hour_, minute_ );
}

