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

#include <kmdcodec.h>

#include <VCardTextBinValue.h>
#include <VCardValue.h>

using namespace VCARD;

TextBinValue::TextBinValue()
	:	Value()
{
}

TextBinValue::TextBinValue(const TextBinValue & x)
	:	Value(x)
{
  mIsBinary_ = x.mIsBinary_;
  mData_ = x.mData_;
  mUrl_ = x.mUrl_;
}

TextBinValue::TextBinValue(const QCString & s)
	:	Value(s)
{
}

	TextBinValue &
TextBinValue::operator = (TextBinValue & x)
{
	if (*this == x) return *this;

  mIsBinary_ = x.mIsBinary_;
  mData_ = x.mData_;
  mUrl_ = x.mUrl_;

	Value::operator = (x);
	return *this;
}

	TextBinValue &
TextBinValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
TextBinValue::operator == (TextBinValue & x)
{
	x.parse();

  if ( mIsBinary_ != x.mIsBinary_ ) return false;
  if ( mData_ != x.mData_ ) return false;
  if ( mUrl_ != x.mUrl_ ) return false;

	return true;
}

TextBinValue::~TextBinValue()
{
}

  TextBinValue *
TextBinValue::clone()
{
  return new TextBinValue( *this );
}

	void
TextBinValue::_parse()
{
}

	void
TextBinValue::_assemble()
{
  if ( mIsBinary_ ) {
    strRep_ = KCodecs::base64Encode( mData_ );
  } else
    strRep_ = mUrl_.utf8();
}

