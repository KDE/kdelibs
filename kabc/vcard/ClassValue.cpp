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

#include <VCardClassValue.h>

#include <VCardValue.h>

#include <kdebug.h>

using namespace VCARD;

ClassValue::ClassValue()
	:	Value()
{
}

ClassValue::ClassValue(const ClassValue & x)
	:	Value(x),
		classType_(x.classType_)
{
}

ClassValue::ClassValue(const QCString & s)
	:	Value(s)
{
}

	ClassValue &
ClassValue::operator = (ClassValue & x)
{
	if (*this == x) return *this;
	
	x.parse();
	
	classType_ = x.classType_;

	Value::operator = (x);
	return *this;
}

	ClassValue &
ClassValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
ClassValue::operator == (ClassValue & x)
{
	x.parse();
	return false;
}

ClassValue::~ClassValue()
{
}

	void
ClassValue::_parse()
{
	if (qstricmp(strRep_, "PUBLIC") == 0)
		classType_ = Public;
		
	else if (qstricmp(strRep_, "PRIVATE") == 0)
		classType_ = Private;
	
	else if (qstricmp(strRep_, "CONFIDENTIAL") == 0)
		classType_ = Confidential;
	
	else classType_ = Other;
}

	void
ClassValue::_assemble()
{
  kdDebug() << "Write type (" << classType_ << "to file." << endl;
	switch (classType_) {
		
		case Public:
			strRep_ = "PUBLIC";
			break;
		
		case Private:
			strRep_ = "PRIVATE";
			break;
			
		case Confidential:
			strRep_ = "CONFIDENTIAL";
			break;
			
		default:
			break;
	}
}

